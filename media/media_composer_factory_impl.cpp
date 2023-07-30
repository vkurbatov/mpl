#include "media_composer_factory_impl.h"
#include "i_media_converter_factory.h"
#include "i_message_frame.h"
#include "i_layout_manager.h"

#include "audio_frame_impl.h"
#include "video_frame_impl.h"

#include "core/i_buffer_collection.h"
#include "core/message_router_impl.h"
#include "core/message_sink_impl.h"
#include "core/time_utils.h"
#include "core/property_writer.h"
#include "core/task_manager_impl.h"
#include "core/adaptive_delay.h"

#include "media_types.h"
#include "image_frame.h"
#include "audio_sample.h"
#include "draw_options.h"
#include "image_builder.h"
#include "message_frame_impl.h"
#include "timestamp_calculator.h"

#include "audio_composer.h"
#include "video_composer.h"

#include "audio_mixer.h"
#include "audio_level.h"
#include "audio_format_helper.h"

#include "tools/base/sync_base.h"

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


constexpr std::size_t default_max_frame_queue = 2;
constexpr timestamp_t default_audio_duration = durations::milliseconds(500);

namespace detail
{

image_frame_t create_image(const i_video_frame& frame)
{
    image_frame_t image(frame.format());

    if (const auto buffer = frame.buffers().get_buffer(main_media_buffer_index))
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

    if (const auto buffer = frame.buffers().get_buffer(main_media_buffer_index))
    {
        sample.sample_data.assign(buffer->data()
                                 , buffer->size()
                                 , false);
    }

    return sample;
}

/*
double animation(double lhs, double rhs, double k, double min_delta)
{
    auto delta = rhs - lhs;
    if (std::abs(delta) < min_delta)
    {
        return rhs;
    }

    return lhs + delta * k;
}

void animation(relative_frame_rect_t& dst_rect
               , const relative_frame_rect_t& src_rect
               , double k
               , double min_delta)
{
    dst_rect.offset.x = animation(dst_rect.offset.x, src_rect.offset.x, k, min_delta);
    dst_rect.offset.y = animation(dst_rect.offset.y, src_rect.offset.y, k, min_delta);
    dst_rect.size.width = animation(dst_rect.size.width, src_rect.size.width, k, min_delta);
    dst_rect.size.height = animation(dst_rect.size.height, src_rect.size.height, k, min_delta);
}*/

}


class media_composer : public i_media_composer
{
    using mutex_t = base::shared_spin_lock;
    template<typename T>
    using shared_lock_t = std::shared_lock<T>;
    template<typename T>
    using lock_t = std::lock_guard<T>;

    using task_queue_t = std::queue<i_task::s_ptr_t>;

    class stream_manager;
    struct composer_params_t;

    class composer_stream : public i_media_stream
            , i_message_sink
            , std::enable_shared_from_this<composer_stream>
    {
    public:
        struct stream_params_t;

    private:

        class audio_track
        {
            using frame_queue_t = std::queue<i_media_frame::s_ptr_t>;
            using compose_stream_ptr_t = audio_composer::i_compose_stream::s_ptr_t;

            mutable mutex_t             m_safe_mutex;


            message_sink_impl           m_message_sink;
            i_media_converter::u_ptr_t  m_media_converter;
            compose_stream_ptr_t        m_compose_stream;


            frame_queue_t               m_frame_queue;
            audio_frame_impl            m_audio_frame;

            timestamp_t                 m_timestamp;
            frame_id_t                  m_frame_id;

            timestamp_calculator        m_timestamp_calculator;

        public:

            audio_track(i_media_converter::u_ptr_t&& media_converter
                        , compose_stream_ptr_t&& compose_stream
                        , const i_audio_format& audio_format)
                : m_message_sink([&](const auto& message_frame) { return on_converter_message(message_frame); })
                , m_media_converter(std::move(media_converter))
                , m_compose_stream(std::move(compose_stream))
                , m_audio_frame(audio_format
                                , 0
                                , 0)
                , m_timestamp(0)
                , m_frame_id(0)
                , m_timestamp_calculator(audio_format.sample_rate())
            {
                if (m_media_converter)
                {
                    m_media_converter->set_sink(&m_message_sink);
                }
            }

            ~audio_track()
            {
                if (m_media_converter)
                {
                    m_media_converter->set_sink(nullptr);
                }
            }

            inline double level() const
            {
                return m_compose_stream->level();
            }
            
            bool push_frame(const i_message_frame& message_frame)
            {
                if (message_frame.frame().media_type() == media_type_t::audio)
                {
                    if (m_media_converter != nullptr)
                    {
                        return m_media_converter->send_message(message_frame);
                    }

                    return on_converter_message(message_frame);
                }

                return false;
            }

            i_media_frame::s_ptr_t pop_frame()
            {
                {
                    lock_t lock(m_safe_mutex);
                    if (!m_frame_queue.empty())
                    {
                        auto frame = std::move(m_frame_queue.front());
                        m_frame_queue.pop();
                        return frame;
                    }
                }

                return nullptr;
            }

            void update_params(const stream_params_t& new_params)
            {
                m_compose_stream->options().volume = new_params.volume;
                m_compose_stream->options().enabled = new_params.audio_enabled;
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


            bool on_converter_frame(const i_media_frame& media_frame)
            {
                if (auto clone_frame = media_frame.clone())
                {
                    lock_t lock(m_safe_mutex);
                    m_frame_queue.emplace(std::move(clone_frame));

                    while (m_frame_queue.size() > 50)
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
                    case message_category_t::frame:
                        return on_converter_frame(static_cast<const i_message_frame&>(message).frame());
                    break;
                    default:;
                }

                return false;
            }

            const i_audio_frame* compose_frame()
            {
                if (auto sample = m_compose_stream->compose_sample())
                {
                    m_audio_frame.smart_buffers().set_buffer(main_media_buffer_index
                                                             , smart_buffer(&sample->sample_data));

                    m_audio_frame.set_timestamp(m_timestamp);
                    m_audio_frame.set_frame_id(m_frame_id);

                    m_timestamp += sample->samples();
                    m_frame_id++;

                    return &m_audio_frame;
                }

                return nullptr;
            }


        };

        class video_track
        {
            using frame_queue_t = std::queue<i_media_frame::s_ptr_t>;
            using compose_stream_ptr_t = video_composer::i_compose_stream::s_ptr_t;
            mutable mutex_t             m_safe_mutex;

            message_sink_impl           m_message_sink;
            i_media_converter::u_ptr_t  m_media_converter;
            compose_stream_ptr_t        m_compose_stream;

            image_frame_t               m_user_image;

            frame_queue_t               m_frame_queue;
            i_media_frame::s_ptr_t      m_last_frame;
            timestamp_t                 m_last_frame_time;

            video_frame_impl            m_video_frame;

            timestamp_t                 m_timestamp;
            frame_id_t                  m_frame_id;
            timestamp_calculator        m_timestamp_calculator;

        public:

            video_track(i_media_converter::u_ptr_t&& media_converter
                        , compose_stream_ptr_t&& compose_stream
                        , const i_video_format& video_format
                        , image_frame_t&& user_image = {})
                : m_message_sink([&](const auto& message_frame) { return on_converter_message(message_frame); })
                , m_media_converter(std::move(media_converter))
                , m_compose_stream(std::move(compose_stream))
                , m_user_image(std::move(user_image))
                , m_last_frame_time(0)
                , m_video_frame(video_format
                                , 0
                                , 0)
                , m_timestamp(0)
                , m_frame_id(0)
                , m_timestamp_calculator(video_sample_rate)
            {
                if (m_media_converter)
                {
                    m_media_converter->set_sink(&m_message_sink);
                }
            }

            ~video_track()
            {
                if (m_media_converter)
                {
                    m_media_converter->set_sink(nullptr);
                }
            }

            bool push_frame(const i_message_frame& message_frame)
            {
                if (m_media_converter)
                {
                    return m_media_converter->send_message(message_frame);
                }

                return on_converter_frame(message_frame.frame());
            }

            i_media_frame::s_ptr_t pop_frame()
            {
                {
                    lock_t lock(m_safe_mutex);
                    if (!m_frame_queue.empty())
                    {
                        m_last_frame = std::move(m_frame_queue.front());
                        m_last_frame_time = core::utils::get_ticks();
                        m_frame_queue.pop();
                    }
                }

                return m_last_frame;
            }

            void update_params(const stream_params_t& new_params)
            {
                m_compose_stream->options().draw_options.target_rect = new_params.draw_options.target_rect;
                m_compose_stream->options().order = new_params.order;
                m_compose_stream->options().animation = new_params.animation;
                m_compose_stream->options().enabled = new_params.video_enabled;
            }

            void clear()
            {
                lock_t lock(m_safe_mutex);
                m_frame_queue = {};
                m_last_frame.reset();
            }

            timestamp_t elapsed_frame_time() const
            {
                return core::utils::get_ticks() - m_last_frame_time;
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

                    m_video_frame.smart_buffers().set_buffer(main_media_buffer_index
                                                             , smart_buffer(&compose_image->image_data));

                    m_timestamp = m_timestamp_calculator.calc_timestamp(frame_time());

                    m_video_frame.set_timestamp(m_timestamp);
                    m_video_frame.set_frame_id(m_frame_id);

                    m_frame_id++;

                    return &m_video_frame;
                }

                return nullptr;
            }

        private:

            bool on_converter_frame(const i_media_frame& media_frame)
            {
                if (auto clone_frame = media_frame.clone())
                {
                    lock_t lock(m_safe_mutex);
                    m_frame_queue.emplace(std::move(clone_frame));

                    while (m_frame_queue.size() > default_max_frame_queue)
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
                    case message_category_t::frame:
                        return on_converter_frame(static_cast<const i_message_frame&>(message).frame());
                    break;
                    default:;
                }

                return false;
            }


        };
    public:
        struct stream_params_t
        {
            std::int32_t            order;
            draw_options_t          draw_options;
            double                  animation;
            timestamp_t             timeout;
            std::string             user_image_path;
            bool                    audio_enabled;
            bool                    video_enabled;
            double                  volume;

            stream_params_t(std::int32_t order = 0
                            , const draw_options_t& draw_options = {}
                            , double animation = 0.0
                            , timestamp_t timeout = 0
                            , const std::string& user_image_path = {}
                            , bool audio_enabled = true
                            , bool video_enabled = true
                            , double volume = 1.0)
                : order(order)
                , draw_options(draw_options)
                , animation(animation)
                , timeout(timeout)
                , user_image_path(user_image_path)
                , audio_enabled(audio_enabled)
                , video_enabled(video_enabled)
                , volume(volume)
            {

            }

            stream_params_t(const i_property& params)
                : stream_params_t()
            {
                load(params);
            }

            bool load(const i_property& params)
            {
                property_reader reader(params);
                return reader.get("order", order)
                        | reader.get("rect", draw_options.target_rect)
                        | reader.get("border.weight", draw_options.border)
                        | reader.get("opacity", draw_options.opacity)
                        | reader.get("label", draw_options.label)
                        | reader.get("elliptic", draw_options.elliptic)
                        | reader.get("animation", animation)
                        | reader.get("timeout", timeout)
                        | reader.get("user_img", user_image_path)
                        | reader.get("audio_enabled", audio_enabled)
                        | reader.get("video_enabled", video_enabled)
                        | reader.get("volume", volume);
            }

            bool save(i_property& params) const
            {
                property_writer writer(params);
                return writer.set("order", order)
                        && writer.set("rect", draw_options.target_rect)
                        && writer.set("border.weight", draw_options.border)
                        && writer.set("opacity", draw_options.opacity)
                        && writer.set("label", draw_options.label)
                        && writer.set("elliptic", draw_options.elliptic)
                        && writer.set("animation", animation)
                        && writer.set("timeout", timeout)
                        && writer.set("user_img", user_image_path, {})
                        && writer.set("audio_enabled", audio_enabled)
                        && writer.set("video_enabled", video_enabled)
                        && writer.set("volume", volume);
            }
        };

    private:

        stream_params_t         m_params;
        stream_manager&         m_manager;

        message_router_impl     m_router;

        audio_track             m_audio_track;
        video_track             m_video_track;

        stream_id_t             m_stream_id;

    public:

        using s_ptr_t = std::shared_ptr<composer_stream>;
        using w_ptr_t = std::weak_ptr<composer_stream>;
        using s_array_t = std::vector<s_ptr_t>;
        using map_t = std::unordered_map<stream_id_t, w_ptr_t>;

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

        static s_ptr_t create(stream_manager& manager
                              , const i_property& stream_params)
        {
            return std::make_shared<composer_stream>(manager
                                                     , stream_params);

        }

        composer_stream(stream_manager& manager
                        , const i_property& stream_params)
            : m_params(stream_params)
            , m_manager(manager)
            , m_audio_track(manager.create_audio_converter()
                            , manager.create_audio_compose_stream(m_params)
                            , manager.composer_params().audio_params.format)
            , m_video_track(manager.create_video_converter()
                            , manager.create_video_compose_stream(m_params)
                            , manager.composer_params().video_params.format
                            , load_image(m_params.user_image_path
                                         , manager.composer_params().video_params.format.format_id()))
            , m_stream_id(manager.next_stream_id())
        {

        }

        ~composer_stream()
        {
            m_manager.on_remove_stream(this);
        }

        void update_draw_options(const relative_frame_rect_t& target_rect = {})
        {
            auto is_layout = !target_rect.is_null();
            const auto& rect = is_layout ? target_rect
                                         : m_params.draw_options.target_rect;

            auto border = m_params.draw_options.border;
            if (is_layout
                    && border == 0
                    && m_audio_track.level() > 0.1)
            {
                border = 2;
            }

            m_video_track.set_layout_params(rect
                                            , border);
        }


        bool push_frame(const i_message_frame& message_frame)
        {
            switch(message_frame.frame().media_type())
            {
                case media_type_t::audio:
                    return m_audio_track.push_frame(message_frame);
                break;
                case media_type_t::video:
                    return m_video_track.push_frame(message_frame);
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
            return !m_params.draw_options.target_rect.is_null();
        }

        inline bool compare(const composer_stream& other) const
        {
            return m_params.order < other.m_params.order
                    || (m_params.order == other.m_params.order
                        && m_stream_id < other.m_stream_id);
        }

        inline bool is_audio_enabled() const
        {
            return m_params.audio_enabled;
        }

        inline bool is_video_enabled() const
        {
            return m_params.video_enabled;
        }

        inline bool is_enabled() const
        {
            return m_params.audio_enabled
                    || m_params.video_enabled;
        }

        void prepare_audio()
        {
            m_audio_track.prepare_inputs();
        }

        void prepare_video()
        {
            m_video_track.prepare_inputs();
        }

        bool feedback_audio()
        {
            if (auto frame = m_audio_track.compose_frame())
            {
                message_frame_ref_impl message_frame(*frame);
                return m_router.send_message(message_frame);
            }

            return false;
        }

        bool feedback_video()
        {
            if (auto frame = m_video_track.compose_frame())
            {
                message_frame_ref_impl message_frame(*frame);
                return m_router.send_message(message_frame);
            }

            return false;
        }

        // i_parametrizable interface
    public:
        bool set_params(const i_property &params) override
        {
            if (m_params.load(params))
            {
                m_audio_track.update_params(m_params);
                m_video_track.update_params(m_params);
                // m_audio_track.set_volume(m_params.volume);
                return true;
            }

            return false;
        }

        bool get_params(i_property &params) const override
        {
            return m_params.save(params);
        }

        // i_media_stream interface
    public:
        stream_id_t stream_id() const override
        {
            return m_stream_id;
        }

        i_message_sink *sink() override
        {
            return this;
        }

        i_message_source *source() override
        {
            return &m_router;
        }

        // i_message_sink interface
    public:
        bool send_message(const i_message &message) override
        {
            switch(message.category())
            {
                case message_category_t::frame:
                    return push_frame(static_cast<const i_message_frame&>(message));
                break;
                default:;
            }

            return false;
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
            , m_audio_composer({ sample_info_t(m_owner.m_composer_params.audio_params.format)
                               , m_owner.m_composer_params.audio_params.frame_size() })
            , m_video_composer(image_info_t(m_owner.m_composer_params.video_params.format))
            , m_stream_ids(0)
        {

        }

        inline const composer_params_t& composer_params() const
        {
            return m_owner.m_composer_params;
        }

        inline const i_layout* get_layout(const composer_stream::s_array_t& streams)
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

        audio_composer::i_compose_stream::s_ptr_t create_audio_compose_stream(const composer_stream::stream_params_t& stream_params)
        {
            audio_composer::compose_options_t options(stream_params.audio_enabled
                                                      , stream_params.volume);

            return m_audio_composer.add_stream(options);
        }

        video_composer::i_compose_stream::s_ptr_t create_video_compose_stream(const composer_stream::stream_params_t& stream_params)
        {
            video_composer::compose_options_t options(stream_params.draw_options
                                                      , stream_params.order
                                                      , stream_params.animation
                                                      , stream_params.video_enabled);

            return m_video_composer.add_stream(options);
        }

        inline stream_id_t next_stream_id()
        {
            return m_stream_ids++;
        }

        composer_stream::s_ptr_t add_stream(const i_property& stream_params)
        {
            if (auto stream = composer_stream::create(*this
                                                      , stream_params))
            {
                lock_t lock(m_safe_mutex);
                m_streams[stream->stream_id()] = stream;
                return stream;
            }

            return nullptr;
        }

        composer_stream::s_ptr_t get_stream(stream_id_t stream_id) const
        {
            shared_lock_t lock(m_safe_mutex);
            if (auto it = m_streams.find(stream_id); it != m_streams.end())
            {
                return it->second.lock();
            }
            return nullptr;
        }

        composer_stream::s_array_t active_streams() const
        {
            composer_stream::s_array_t array;
            {
                // shared_lock_t lock(m_safe_mutex);
                for (const auto& s : m_streams)
                {
                    if (auto stream = s.second.lock())
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
        auto tp = mpl::core::utils::get_ticks();
        while(m_started)
        {
            timestamp_t video_delay = durations::second / m_composer_params.video_params.format.frame_rate();

            m_stream_manager.compose_video();
            m_video_compose_delay.wait(video_delay);

            frames++;

            auto dt = mpl::core::utils::get_ticks() - tp;

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
    i_media_stream::s_ptr_t add_stream(const i_property &stream_property) override
    {
        return m_stream_manager.add_stream(stream_property);
    }

    i_media_stream::s_ptr_t get_stream(stream_id_t stream_id) const override
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
