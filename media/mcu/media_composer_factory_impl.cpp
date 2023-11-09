#include "media_composer_factory_impl.h"
#include "media/i_media_converter_factory.h"
#include "media/i_layout_manager.h"
#include "media/i_media_track.h"

#include "media/audio_frame_impl.h"
#include "media/video_frame_impl.h"

#include "core/i_buffer_collection.h"
#include "utils/message_router_impl.h"
#include "utils/message_sink_impl.h"
#include "utils/time_utils.h"
#include "utils/property_writer.h"
#include "utils/task_manager_impl.h"
#include "utils/adaptive_delay.h"
#include "utils/option_helper.h"

#include "media/media_module_types.h"
#include "media/media_types.h"
#include "media/media_option_types.h"

#include "media/image_frame.h"
#include "media/audio_sample.h"
#include "media/draw_options.h"
#include "media/image_builder.h"
#include "media/timestamp_calculator.h"

#include "compose_stream_params.h"

#include "media/audio_mixer.h"
#include "media/audio_level.h"
#include "media/audio_format_helper.h"

#include "audio_composer.h"
#include "video_composer.h"

#include "tools/utils/sync_base.h"

#include <future>
#include <shared_mutex>
#include <thread>
#include <queue>
#include <unordered_map>
#include <cmath>

#include <cstring>

#include <iostream>

namespace mpl::media
{


constexpr std::size_t default_max_video_frame_queue = 2;
constexpr std::size_t default_max_audio_frame_queue = 50;
constexpr timestamp_t default_audio_duration = durations::milliseconds(500);

namespace detail
{

image_frame_t create_image(const i_video_frame& frame)
{
    image_frame_t image(frame.format());

    if (const auto buffer = frame.data().get_buffer(media_buffer_index))
    {
        image.image_data.assign(buffer->data()
                                , buffer->size()
                                , false);
    }

    return image;
}

audio_sample_t create_sample(const i_audio_frame& frame)
{
    audio_sample_t sample(frame.format());

    if (const auto buffer = frame.data().get_buffer(media_buffer_index))
    {
        sample.sample_data.assign(buffer->data()
                                 , buffer->size()
                                 , false);
    }

    return sample;
}

}


class media_composer : public i_media_composer
{
    using mutex_t = pt::utils::shared_spin_lock;
    template<typename T>
    using shared_lock_t = std::shared_lock<T>;
    template<typename T>
    using lock_t = std::lock_guard<T>;

    using task_queue_t = std::queue<i_task::s_ptr_t>;

    class stream_manager;
    struct composer_params_t;

    class composer_stream : public i_media_stream
            , i_message_sink
    {
    private:

        class media_track : public i_media_track
        {
        protected:
            using frame_queue_t = std::queue<i_message::s_ptr_t>;

            mutable mutex_t             m_safe_mutex;
            composer_stream&            m_owner;

            message_sink_impl           m_message_sink;
            i_media_converter::u_ptr_t  m_media_converter;

            frame_queue_t               m_frame_queue;
            std::size_t                 m_max_queue_size;

            timestamp_t                 m_timestamp;
            frame_id_t                  m_frame_id;

            bool                        m_enabled;

        public:
            media_track(composer_stream& owner
                        , i_media_converter::u_ptr_t&& media_converter
                        , std::size_t max_queue_size)
                : m_owner(owner)
                , m_message_sink([&](const auto& message_frame) { return on_converter_message(message_frame); })
                , m_media_converter(std::move(media_converter))
                , m_max_queue_size(max_queue_size)
                , m_timestamp(0)
                , m_frame_id(0)
                , m_enabled(false)
            {

            }

            ~media_track()
            {
                if (m_media_converter)
                {
                    m_media_converter->set_sink(nullptr);
                }
            }

            mutex_t& safe_mutex() const
            {
                return m_owner.safe_mutex();
            }

            inline compose_stream_params_t& stream_params() const
            {
                return m_owner.m_params;
            }

            bool push_frame(const i_media_frame& media_frame)
            {
                if (m_media_converter)
                {
                    return m_media_converter->send_message(media_frame);
                }

                return on_converter_frame(media_frame);
            }


            bool on_converter_frame(const i_media_frame& media_frame)
            {
                if (auto clone_frame = media_frame.clone())
                {
                    lock_t lock(m_safe_mutex);
                    m_frame_queue.emplace(std::move(clone_frame));

                    while (m_frame_queue.size() > m_max_queue_size)
                    {
                        m_frame_queue.pop();
                    }

                    return true;
                }
                return false;
            }

            bool on_converter_message(const i_message& message)
            {
                switch(message.category())
                {
                    case message_category_t::data:
                        if (static_cast<const i_message_data&>(message).module_id() == media_module_id
                                && static_cast<const i_message_media_data&>(message).data_type() == media_data_type_t::frame)
                        {
                            return on_converter_frame(static_cast<const i_media_frame&>(message));
                        }
                    break;
                    default:;
                }

                return false;
            }

            // i_media_track interface
        public:
            const i_media_stream &media_stream() const override
            {
                return m_owner;
            }
        };

        class audio_track : public media_track
        {
            using compose_stream_ptr_t = audio_composer::i_compose_stream::u_ptr_t;

            compose_stream_ptr_t        m_compose_stream;
            audio_frame_impl            m_audio_frame;

            timestamp_calculator        m_timestamp_calculator;

        public:

            audio_track(composer_stream& owner
                        , i_media_converter::u_ptr_t&& media_converter
                        , compose_stream_ptr_t&& compose_stream
                        , const i_audio_format& audio_format)
                : media_track(owner
                              , std::move(media_converter)
                              , default_max_audio_frame_queue)
                , m_compose_stream(std::move(compose_stream))
                , m_audio_frame(audio_format
                                , 0
                                , 0)
                , m_timestamp_calculator(audio_format.sample_rate())
            {
                m_audio_frame.set_stream_id(owner.m_stream_id);
                m_audio_frame.set_track_id(default_audio_track_id);

                if (m_media_converter)
                {
                    m_media_converter->set_sink(&m_message_sink);
                }
            }

            inline double level() const
            {
                return m_compose_stream->level();
            }

            i_media_frame::s_ptr_t pop_frame()
            {
                {
                    lock_t lock(m_safe_mutex);
                    if (!m_frame_queue.empty())
                    {
                        auto frame = std::move(m_frame_queue.front());
                        m_frame_queue.pop();
                        return std::static_pointer_cast<i_media_frame>(frame);
                    }
                }

                return nullptr;
            }

            const compose_audio_track_params_t& track_params() const
            {
                return stream_params().audio_track;
            }

            inline void update_params()
            {
                m_compose_stream->options().volume = track_params().volume;
                m_compose_stream->options().enabled = track_params().enabled;
            }

            void prepare_inputs()
            {
                while(auto frame = pop_frame())
                {
                    if (frame->media_type() == media_type_t::audio)
                    {
                        auto sample = detail::create_sample(static_cast<const i_audio_frame&>(*frame));
                        m_compose_stream->push_stream_sample(std::move(sample));
                    }
                }
            }

            const i_audio_frame* compose_frame()
            {
                if (auto sample = m_compose_stream->compose_sample())
                {
                    m_audio_frame.smart_buffers().set_buffer(media_buffer_index
                                                             , smart_buffer(&sample->sample_data));
                    m_audio_frame.set_timestamp(m_timestamp);
                    m_audio_frame.set_frame_id(m_frame_id);

                    m_timestamp += sample->samples();
                    m_frame_id++;

                    return &m_audio_frame;
                }

                return nullptr;
            }



            // i_media_track interface
        public:
            track_id_t track_id() const override
            {
                return m_audio_frame.track_id();
            }

            const i_media_format &format() const override
            {
                return m_audio_frame.format();
            }

            // i_media_track interface
        public:
            bool is_enabled() const override
            {
                return m_compose_stream->options().enabled;
            }

            bool set_enabled(bool enabled) override
            {
                m_owner.m_params.audio_track.enabled = enabled;
                m_compose_stream->options().enabled = m_owner.m_params.audio_track.enabled;
                return true;
            }

            // i_parametrizable interface
        public:
            bool set_params(const i_property &params) override
            {
                lock_t lock(safe_mutex());
                if (utils::property::deserialize(m_owner.m_params.audio_track
                                                    , params))
                {
                    update_params();
                    return true;
                }

                return false;
            }

            bool get_params(i_property &params) const override
            {
                shared_lock_t lock(safe_mutex());
                return utils::property::serialize(m_owner.m_params.audio_track
                                                    , params);
            }

            // i_media_track interface
        public:
            std::string name() const override
            {
                return m_owner.m_params.audio_track.name;
            }
        };

        class video_track : public media_track
        {
            using compose_stream_ptr_t = video_composer::i_compose_stream::u_ptr_t;

            compose_stream_ptr_t        m_compose_stream;
            image_frame_t               m_user_image;

            i_media_frame::s_ptr_t      m_last_frame;
            timestamp_t                 m_last_frame_time;

            video_frame_impl            m_video_frame;
            timestamp_calculator        m_timestamp_calculator;

        public:

            video_track(composer_stream& owner
                        , i_media_converter::u_ptr_t&& media_converter
                        , compose_stream_ptr_t&& compose_stream
                        , const i_video_format& video_format
                        , image_frame_t&& user_image = {})
                : media_track(owner
                              , std::move(media_converter)
                              , default_max_video_frame_queue)
                , m_compose_stream(std::move(compose_stream))
                , m_user_image(std::move(user_image))
                , m_last_frame_time(0)
                , m_video_frame(video_format
                                , 0
                                , 0)
                , m_timestamp_calculator(video_sample_rate)
            {
                m_video_frame.set_stream_id(m_owner.m_stream_id);
                m_video_frame.set_track_id(default_video_track_id);

                if (m_media_converter)
                {
                    m_media_converter->set_sink(&m_message_sink);
                }
            }

            i_media_frame::s_ptr_t pop_frame()
            {
                {
                    lock_t lock(m_safe_mutex);
                    if (!m_frame_queue.empty())
                    {
                        m_last_frame = std::move(std::static_pointer_cast<i_media_frame>(m_frame_queue.front()));
                        m_last_frame_time = utils::time::get_ticks();
                        m_frame_queue.pop();
                    }
                }

                return m_last_frame;
            }

            const compose_video_track_params_t& track_params() const
            {
                return stream_params().video_track;
            }

            void update_params()
            {
                m_compose_stream->options().draw_options.target_rect = track_params().draw_options.target_rect;
                m_compose_stream->options().order = stream_params().order;
                m_compose_stream->options().animation = track_params().animation;
                m_compose_stream->options().enabled = track_params().enabled;
            }

            inline void clear()
            {
                lock_t lock(m_safe_mutex);
                m_frame_queue = {};
                m_last_frame.reset();
            }

            inline timestamp_t elapsed_frame_time() const
            {
                return utils::time::get_ticks() - m_last_frame_time;
            }

            inline void set_layout_params(const relative_frame_rect_t& rect
                                          , std::uint32_t border = 0)
            {
                m_compose_stream->options().draw_options.target_rect = rect;
                m_compose_stream->options().draw_options.border = border;
            }

            bool prepare_inputs()
            {
                if (auto frame = pop_frame())
                {
                    if (frame->media_type() == media_type_t::video)
                    {
                        image_frame_t image = detail::create_image(static_cast<const i_video_frame&>(*frame));
                        if (image.is_valid())
                        {
                            return m_compose_stream->push_stream_image(std::move(image));
                        }
                    }
                }
                else
                {
                    if (m_user_image.is_valid())
                    {
                        return m_compose_stream->push_stream_image(image_frame_t(m_user_image));
                    }
                }

                return false;
            }

            inline timestamp_t frame_time() const
            {
                auto frame_rate = m_video_frame.format().frame_rate();

                return frame_rate > 0
                        ? video_sample_rate / frame_rate
                        : 0;
            }

            const i_video_frame* compose_frame()
            {
                if (auto compose_image = m_compose_stream->compose_image())
                {

                    m_video_frame.smart_buffers().set_buffer(media_buffer_index
                                                             , smart_buffer(&compose_image->image_data));

                    m_timestamp = m_timestamp_calculator.calc_timestamp(frame_time());

                    m_video_frame.set_timestamp(m_timestamp);
                    m_video_frame.set_frame_id(m_frame_id);

                    m_frame_id++;

                    return &m_video_frame;
                }

                return nullptr;
            }

            // i_media_track interface
        public:
            track_id_t track_id() const override
            {
                return m_video_frame.track_id();
            }

            const i_media_format &format() const override
            {
                return m_video_frame.format();
            }

            // i_media_track interface
        public:
            bool is_enabled() const override
            {
                return m_compose_stream->options().enabled;
            }

            bool set_enabled(bool enabled) override
            {
                m_owner.m_params.video_track.enabled = enabled;
                m_compose_stream->options().enabled = m_owner.m_params.video_track.enabled;
                return true;
            }

            // i_parametrizable interface
        public:
            bool set_params(const i_property &params) override
            {
                lock_t lock(safe_mutex());
                if (utils::property::deserialize(m_owner.m_params.video_track
                                                    , params))
                {
                    update_params();
                    return true;
                }

                return false;
            }

            bool get_params(i_property &params) const override
            {
                shared_lock_t lock(safe_mutex());
                return utils::property::serialize(m_owner.m_params.video_track
                                                   , params);
            }

            std::string name() const override
            {
                return m_owner.m_params.video_track.name;
            }
        };
    private:

        compose_stream_params_t m_params;
        stream_manager&         m_manager;

        message_router_impl     m_router;

        stream_id_t             m_stream_id;

        audio_track             m_audio_track;
        video_track             m_video_track;

    public:

        using u_ptr_t = std::unique_ptr<composer_stream>;
        using s_ptr_t = std::shared_ptr<composer_stream>;
        using w_ptr_t = std::weak_ptr<composer_stream>;
        using s_array_t = std::vector<s_ptr_t>;
        using r_array_t = std::vector<composer_stream*>;
        using map_t = std::unordered_map<stream_id_t, composer_stream*>;

        static image_frame_t load_image(const std::string& user_image_path
                                        , video_format_id_t format_id)
        {
            image_frame_t image;
            if (image.load(user_image_path
                           , format_id))
            {
                return image;
            }

            return {};
        }

        static u_ptr_t create(stream_manager& manager
                              , const i_property& stream_property)
        {
            compose_stream_params_t stream_params;
            if (utils::property::deserialize(stream_params
                                             , stream_property))
            {
                return std::make_unique<composer_stream>(manager
                                                         , std::move(stream_params)
                                                         );
            }

            return nullptr;

        }

        composer_stream(stream_manager& manager
                        , compose_stream_params_t&& stream_params)
            : m_params(std::move(stream_params))
            , m_manager(manager)
            , m_stream_id(manager.next_stream_id())
            , m_audio_track(*this
                            , manager.create_audio_converter()
                            , manager.create_audio_compose_stream(m_params)
                            , manager.composer_params().audio_params.format)
            , m_video_track(*this
                            , manager.create_video_converter()
                            , manager.create_video_compose_stream(m_params)
                            , manager.composer_params().video_params.format
                            , load_image(m_params.video_track.user_image_path
                            , manager.composer_params().video_params.format.format_id()))
        {

        }

        ~composer_stream()
        {
            m_manager.on_remove_stream(this);
        }

        mutex_t& safe_mutex() const
        {
            return m_manager.m_safe_mutex;
        }

        void update_draw_options(const relative_frame_rect_t& target_rect = {})
        {
            auto is_layout = !target_rect.is_null();
            const auto& rect = is_layout ? target_rect
                                         : m_params.video_track.draw_options.target_rect;

            auto border = m_params.video_track.draw_options.border;
            if (is_layout
                    && border == 0
                    && m_audio_track.level() > 0.1)
            {
                border = 2;
            }

            m_video_track.set_layout_params(rect
                                            , border);
        }


        inline bool push_frame(const i_media_frame& media_frame)
        {
            switch(media_frame.media_type())
            {
                case media_type_t::audio:
                    return m_audio_track.push_frame(media_frame);
                break;
                case media_type_t::video:
                    return m_video_track.push_frame(media_frame);
                break;
                default:;
            }

            return false;
        }

        inline std::int32_t order() const
        {
            return m_params.order;
        }

        inline bool is_custom() const
        {
            return !m_params.video_track.draw_options.target_rect.is_null();
        }

        inline bool compare(const composer_stream& other) const
        {
            return m_params.order < other.m_params.order
                    || (m_params.order == other.m_params.order
                        && m_stream_id < other.m_stream_id);
        }

        inline bool is_audio_enabled() const
        {
            return m_params.audio_track.enabled;
        }

        inline bool is_video_enabled() const
        {
            return m_params.video_track.enabled;
        }

        inline bool is_enabled() const
        {
            return m_params.audio_track.enabled
                    || m_params.video_track.enabled;
        }

        inline void prepare_audio()
        {
            m_audio_track.prepare_inputs();
        }

        void prepare_video()
        {
            m_video_track.prepare_inputs();
        }

        inline bool feedback_audio()
        {
            if (auto frame = m_audio_track.compose_frame())
            {
                return m_router.send_message(*frame);
            }

            return false;
        }

        inline bool feedback_video()
        {
            if (auto frame = m_video_track.compose_frame())
            {
                return m_router.send_message(*frame);
            }

            return false;
        }

        // i_parametrizable interface
    public:
        bool set_params(const i_property &params) override
        {
            lock_t lock(safe_mutex());
            if (utils::property::deserialize(m_params
                                             , params))
            {
                m_audio_track.update_params();
                m_video_track.update_params();
                return true;
            }

            return false;
        }

        bool get_params(i_property &params) const override
        {
            shared_lock_t lock(safe_mutex());
            return utils::property::serialize(m_params
                                              , params);
        }

        // i_media_stream interface
    public:
        stream_id_t stream_id() const override
        {
            return m_stream_id;
        }

        std::string name() const override
        {
            return m_params.name;
        }

        i_message_sink *sink(std::size_t index) override
        {
            if (index == 0)
            {
                return this;
            }

            return nullptr;
        }

        i_message_source *source(std::size_t index) override
        {
            if (index == 0)
            {
                return &m_router;
            }

            return nullptr;
        }

        // i_message_sink interface
    public:
        bool send_message(const i_message &message) override
        {
            switch(message.category())
            {
                case message_category_t::data:
                    if (static_cast<const i_message_data&>(message).module_id() == media_module_id
                            && static_cast<const i_message_media_data&>(message).data_type() == media_data_type_t::frame)
                    {
                        return push_frame(static_cast<const i_media_frame&>(message));
                    }
                break;
                default:;
            }

            return false;
        }

        // i_media_stream interface
    public:
        i_media_track *get_track(track_id_t track_id) override
        {
            switch(track_id)
            {
                case default_audio_track_id:
                    return &m_audio_track;
                break;
                case default_video_track_id:
                    return &m_video_track;
                break;
                default:;
            }

            return nullptr;
        }
    };

    class stream_manager
    {
        mutable mutex_t             m_safe_mutex;
        media_composer&             m_owner;
        i_media_converter_factory&  m_converter_factory;
        i_layout_manager&           m_layout_manager;

        audio_composer              m_audio_composer;
        video_composer              m_video_composer;

        composer_stream::map_t      m_streams;

        stream_id_t                 m_stream_ids;

        friend class composer_stream;

    public:
        stream_manager(media_composer& owner
                       , i_media_converter_factory& converter_factory
                       , i_layout_manager& layout_manager)
            : m_owner(owner)
            , m_converter_factory(converter_factory)
            , m_layout_manager(layout_manager)
            , m_audio_composer({ audio_info_t(m_owner.m_composer_params.audio_params.format)
                               , m_owner.m_composer_params.audio_params.frame_size() })
            , m_video_composer(video_info_t(m_owner.m_composer_params.video_params.format))
            , m_stream_ids(0)
        {

        }

        inline const composer_params_t& composer_params() const
        {
            return m_owner.m_composer_params;
        }

        inline const i_layout* get_layout(const composer_stream::r_array_t& streams)
        {
            std::size_t result = 0;
            for (const auto& s: streams)
            {
                if (!s->is_custom())
                {
                    result++;
                }
            }

            return m_layout_manager.query_layout(result);
        }

        i_media_converter::u_ptr_t create_video_converter()
        {
            video_format_impl format(m_owner.m_composer_params.video_params.format);
            format.set_width(0);
            format.set_height(0);

            if (auto params = format.get_params("format"))
            {
                property_writer writer(*params);
                writer.set("transcode_async", true);
                return m_converter_factory.create_converter(*params);
            }

            return nullptr;
        }

        i_media_converter::u_ptr_t create_audio_converter()
        {
            if (auto params = m_owner.m_composer_params.audio_params.format.get_params("format"))
            {
                return m_converter_factory.create_converter(*params);
            }

            return nullptr;
        }

        audio_composer::i_compose_stream::u_ptr_t create_audio_compose_stream(const compose_stream_params_t& stream_params)
        {
            audio_composer::compose_options_t options(stream_params.audio_track.enabled
                                                      , stream_params.audio_track.volume);

            return m_audio_composer.add_stream(options);
        }

        video_composer::i_compose_stream::u_ptr_t create_video_compose_stream(const compose_stream_params_t& stream_params)
        {
            video_composer::compose_options_t options(stream_params.video_track.draw_options
                                                      , stream_params.order
                                                      , stream_params.video_track.animation
                                                      , stream_params.video_track.enabled);

            return m_video_composer.add_stream(options);
        }

        inline stream_id_t next_stream_id()
        {
            return m_stream_ids++;
        }

        composer_stream::u_ptr_t add_stream(const i_property& stream_params)
        {
            if (auto stream = composer_stream::create(*this
                                                      , stream_params))
            {
                lock_t lock(m_safe_mutex);
                m_streams[stream->stream_id()] = stream.get();
                return stream;
            }

            return nullptr;
        }

        composer_stream* get_stream(stream_id_t stream_id) const
        {
            shared_lock_t lock(m_safe_mutex);
            if (auto it = m_streams.find(stream_id); it != m_streams.end())
            {
                return it->second;
            }
            return nullptr;
        }

        composer_stream::r_array_t active_streams() const
        {
            composer_stream::r_array_t array;
            {
                // shared_lock_t lock(m_safe_mutex);
                for (const auto& s : m_streams)
                {
                    if (auto stream = s.second)
                    {
                        if (stream->is_enabled())
                        {
                            array.emplace_back(std::move(stream));
                        }
                    }
                }
            }

            return array;
        }

        void compose_video()
        {
            shared_lock_t lock(m_safe_mutex);
            auto streams = active_streams();
            if (auto layouts = get_layout(streams))
            {
                auto idx = 0;
                for (auto& s : streams)
                {
                    s->prepare_video();
                    if (!s->is_custom())
                    {
                        s->update_draw_options(layouts->get_rect(idx++));
                    }
                    else
                    {
                        s->update_draw_options();
                    }
                }
            }

            m_video_composer.compose();
            for (const auto& s : streams)
            {
                s->feedback_video();
            }

        }

        void compose_audio()
        {
            shared_lock_t lock(m_safe_mutex);
            auto streams = active_streams();
            for (const auto& s : streams)
            {
                s->prepare_audio();
            }

            m_audio_composer.compose();
            for (const auto& s : streams)
            {
                s->feedback_audio();
            }

        }

    private:

        void on_remove_stream(composer_stream* stream)
        {
            lock_t lock(m_safe_mutex);
            m_streams.erase(stream->stream_id());
        }

    };

    struct composer_params_t
    {
        struct audio_params_t
        {
            static constexpr timestamp_t min_duraion = durations::milliseconds(10);

            audio_format_impl   format;
            timestamp_t         duration;

            inline std::size_t frame_size() const
            {
                return audio_format_helper(format).samples_from_duration(duration);
            }

            inline bool has_enabled() const
            {
                return duration >= min_duraion
                        && format.is_valid()
                        && format.is_convertable();
            }
        };

        struct video_params_t
        {
            video_format_impl   format;

            inline bool has_enabled() const
            {
                if (format.is_valid()
                        && format.frame_rate() > 0)
                {
                    switch(format.format_id())
                    {
                        case video_format_id_t::rgb24:
                        case video_format_id_t::rgba32:
                        case video_format_id_t::bgr24:
                        case video_format_id_t::bgra32:
                            return true;
                        break;
                        default:;
                    }
                }

                return false;
            }
        };

        audio_params_t      audio_params;
        video_params_t      video_params;


        composer_params_t(const i_property& params)
        {
            load(params);
        }

        bool load(const i_property& params)
        {
            property_reader reader(params);

            bool result = false;


            if (auto audio_format_params = reader["audio_params.format"])
            {
                if (!audio_params.format.set_params(*audio_format_params))
                {
                    return false;
                }

                result = true;
            }

            if (auto video_format_params = reader["video_params.format"])
            {
                if (!video_params.format.set_params(*video_format_params))
                {
                    return false;
                }

                result = true;
            }

            reader.get("audio_params.duration", audio_params.duration);


            return result;
        }

        bool save(i_property& params) const
        {
            property_writer writer(params);

            return writer.set("audio_params.format", audio_params.format)
                    && writer.set("video_params.format", video_params.format)
                    && writer.set("audio_params.duration", audio_params.duration);

        }

        inline bool has_video() const
        {
            return video_params.has_enabled();
        }

        inline bool has_audio() const
        {
            return audio_params.has_enabled();
        }

        inline bool is_valid() const
        {
            return has_video()
                    || has_audio();
        }
    };

    composer_params_t           m_composer_params;


    stream_manager              m_stream_manager;



    adaptive_delay              m_video_compose_delay;
    adaptive_delay              m_audio_compose_delay;

    std::thread                 m_video_thread;
    std::thread                 m_audio_thread;

    bool                        m_started;

public:
    using u_ptr_t = std::unique_ptr<media_composer>;

    static u_ptr_t create(i_media_converter_factory& converter_factory
                          , i_layout_manager& layout_manager
                          , const i_property &params)
    {
        composer_params_t composer_params(params);
        if (composer_params.is_valid())
        {

            return std::make_unique<media_composer>(converter_factory
                                                    , layout_manager
                                                    , std::move(composer_params));
        }

        return nullptr;
    }

    media_composer(i_media_converter_factory& converter_factory
                   , i_layout_manager& layout_manager
                   , composer_params_t&& composer_params)
        : m_composer_params(std::move(composer_params))
        , m_stream_manager(*this
                           , converter_factory
                           , layout_manager)
        , m_started(false)
    {

    }

    ~media_composer()
    {
        media_composer::stop();
    }

    void video_compose_proc()
    {
        m_video_compose_delay.reset();
        std::size_t frames = 0;
        auto tp = utils::time::get_ticks();
        while(m_started)
        {
            timestamp_t video_delay = durations::second / m_composer_params.video_params.format.frame_rate();

            m_stream_manager.compose_video();
            m_video_compose_delay.wait(video_delay);

            frames++;

            auto dt = utils::time::get_ticks() - tp;

            auto fps = durations::seconds(frames) / dt;


            std::clog << "fps: " << fps << std::endl;
        }
    }

    void audio_compose_proc()
    {
        m_audio_compose_delay.reset();
        while(m_started)
        {
            m_stream_manager.compose_audio();
            // m_audio_composer.compose_streams(streams);

            m_audio_compose_delay.wait(m_composer_params.audio_params.duration);
        }
    }


    // i_parametrizable interface
public:
    bool set_params(const i_property &params) override
    {
        return false;
    }

    bool get_params(i_property &params) const override
    {
        return m_composer_params.save(params);
    }

    // i_media_composer interface
public:
    i_media_stream::u_ptr_t add_stream(const i_property &stream_property) override
    {
        return m_stream_manager.add_stream(stream_property);
    }

    i_media_stream* get_stream(stream_id_t stream_id) override
    {
        return m_stream_manager.get_stream(stream_id);
    }

    bool start() override
    {
        if (!m_started)
        {
            if (m_composer_params.is_valid())
            {
                m_started = true;
                m_audio_thread = std::thread([&]{ audio_compose_proc(); });
                m_video_thread = std::thread([&]{ video_compose_proc(); });

                return true;
            }
        }

        return false;
    }

    bool stop() override
    {
        if (m_started)
        {
            m_started = false;
            if (m_audio_thread.joinable())
            {
                m_audio_thread.join();
            }

            if (m_video_thread.joinable())
            {
                m_video_thread.join();
            }

            return true;
        }

        return false;
    }

    bool is_started() const override
    {
        return m_started;
    }

};

media_composer_factory_impl::u_ptr_t media_composer_factory_impl::create(i_media_converter_factory &media_converter_factory
                                                                         , i_layout_manager& layout_manager)
{
    return std::make_unique<media_composer_factory_impl>(media_converter_factory
                                                         , layout_manager);
}

media_composer_factory_impl::media_composer_factory_impl(i_media_converter_factory &media_converter_factory
                                                         , i_layout_manager& layout_manager)
    : m_media_converter_factory(media_converter_factory)
    , m_layout_manager(layout_manager)
{

}

i_media_composer::u_ptr_t media_composer_factory_impl::create_composer(const i_property &params)
{
    return media_composer::create(m_media_converter_factory
                                  , m_layout_manager
                                  , params);
}


}
