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
#include "video_image_builder.h"
#include "message_frame_impl.h"

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
    image_frame_t image(frame.format().format_id()
                        , { frame.format().width(), frame.format().height() });

    if (const auto buffer = frame.buffers().get_buffer(main_media_buffer_index))
    {
        image.image_data.assign(buffer->data()
                                , buffer->size()
                                , false);
    }

    return image;
}

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

        class audio_track
        {
            mutable mutex_t             m_safe_mutex;

            message_sink_impl           m_message_sink;
            i_media_converter::u_ptr_t  m_media_converter;

            audio_mixer                 m_input_mixer;
            audio_mixer                 m_output_mixer;
            audio_level                 m_audio_level;

        public:

            audio_track(i_media_converter::u_ptr_t&& media_converter
                        , const i_audio_format& audio_format
                        , std::size_t buffer_size)
                : m_message_sink([&](const auto& message_frame) { return on_converter_message(message_frame); })
                , m_media_converter(std::move(media_converter))
                , m_input_mixer(audio_format
                                , buffer_size)
                , m_output_mixer(audio_format
                                 , buffer_size)
                , m_audio_level(audio_level::config_t{})
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

            bool push_audio_data(const void* sample_data
                               , std::size_t samples
                               , double level = 1.0)
            {
                lock_t lock(m_safe_mutex);
                if (m_input_mixer.push_data(sample_data
                                            , samples
                                            , level) == samples)
                {
                    m_audio_level.push_frame(m_input_mixer.format()
                                             , sample_data
                                             , samples);

                    return true;
                }

                return false;
            }

            bool mix(void* data
                     , std::size_t samples)
            {
                lock_t lock(m_safe_mutex);

                if (is_overrun())
                {
                    m_input_mixer.reset();
                    m_output_mixer.reset();

                    return false;
                }

                if (m_input_mixer.copy_data(m_output_mixer
                                            , samples) == samples)
                {
                    return m_input_mixer.pop_data(data
                                                , samples
                                                , audio_mixer::mix_method_t::mix) == samples;
                }

                return false;
            }

            inline bool demix(void* data
                              , std::size_t samples)
            {
                return m_output_mixer.pop_data(data
                                                , samples
                                                , audio_mixer::mix_method_t::demix) == samples;
            }

            inline void reset()
            {
                m_input_mixer.reset();
                m_output_mixer.reset();
            }

            inline bool is_overrun() const
            {
                return m_input_mixer.overrun() > 0
                        || m_output_mixer.overrun() > 0;
            }

            inline double level() const
            {
                return m_audio_level.level();
            }

            bool on_converter_frame(const i_media_frame& media_frame)
            {
                if (media_frame.media_type() == media_type_t::audio)
                {
                    const i_audio_frame& audio_frame = static_cast<const i_audio_frame&>(media_frame);
                    if (audio_frame.format().is_compatible(m_input_mixer.format()))
                    {
                        if (auto buffer = audio_frame.buffers().get_buffer(main_media_buffer_index))
                        {
                            auto audio_data = buffer->data();
                            auto audio_samples = audio_format_helper(audio_frame.format()).samples_from_size(buffer->size());

                            return push_audio_data(audio_data
                                                    , audio_samples);
                        }

                    }
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

        class video_track
        {
            using frame_queue_t = std::queue<i_media_frame::s_ptr_t>;
            mutable mutex_t             m_safe_mutex;

            message_sink_impl           m_message_sink;
            i_media_converter::u_ptr_t  m_media_converter;

            image_frame_t               m_user_image;
            video_image_builder         m_image_builder;

            frame_queue_t               m_frame_queue;
            i_media_frame::s_ptr_t      m_last_frame;
            timestamp_t                 m_last_frame_time;

        public:

            video_track(i_media_converter::u_ptr_t&& media_converter
                        ,image_frame_t&& user_image = {})
                : m_message_sink([&](const auto& message_frame) { return on_converter_message(message_frame); })
                , m_media_converter(std::move(media_converter))
                , m_user_image(std::move(user_image))
                , m_image_builder({}
                                  , nullptr)
                , m_last_frame_time(0)
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


            bool compose(const draw_options_t& options
                         , image_frame_t* output_image)
            {
                if (!options.target_rect.size.is_null())
                {
                    m_image_builder.set_output_frame(output_image);
                    if (auto video_frame = pop_frame())
                    {
                        auto image = detail::create_image(static_cast<const i_video_frame&>(*video_frame));
                        if (image.is_valid())
                        {
                            return m_image_builder.draw_image_frame(image
                                                                    , options);
                        }
                    }
                    else if (m_user_image.is_valid())
                    {
                        return m_image_builder.draw_image_frame(m_user_image
                                                                , options);
                    }
                }

                return false;
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

        struct stream_params_t
        {
            std::int32_t            order;
            draw_options_t          draw_options;
            timestamp_t             timeout;
            std::string             user_image_path;

            stream_params_t(std::int32_t order = 0
                            , const draw_options_t& draw_options = {})
                : order(order)
                , draw_options(draw_options)
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
                        | reader.get("timeout", timeout)
                        | reader.get("user_img", user_image_path);
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
                        && writer.set("timeout", timeout)
                        && writer.set("user_img", user_image_path, {});
            }
        };

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
            , m_audio_track(manager.create_converter(media_type_t::audio)
                            , manager.composer_params().audio_params.format
                            , manager.composer_params().audio_params.format.sample_rate() / 2)
            , m_video_track(manager.create_converter(media_type_t::video)
                            , load_image(m_params.user_image_path
                                         , manager.composer_params().video_params.format.format_id()))
            , m_stream_id(manager.next_stream_id())
        {

        }

        ~composer_stream()
        {
            m_manager.on_remove_stream(this);
        }

        bool feedback_frame(const i_message_frame& message_frame)
        {
            return m_router.send_message(message_frame);
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

        bool compose_video(image_frame_t* compose_frame
                           , const relative_frame_rect_t& target_rect = {})
        {
            draw_options_t options = m_params.draw_options;
            if (!target_rect.is_null())
            {
                options.target_rect = target_rect;
            }

            if (options.border == 0)
            {
                if (m_audio_track.level() > 0.1)
                {
                    options.border = 2;
                }
            }

            return m_video_track.compose(options
                                         , compose_frame);
        }

        inline bool mix_audio(void* data
                             , std::size_t samples)
        {
            return m_audio_track.mix(data
                                     , samples);
        }

        bool compose_audio(const i_audio_frame& compose_frame)
        {
            if (auto frame_buffer = compose_frame.buffers().get_buffer(main_media_buffer_index))
            {

                smart_buffer stream_buffer(frame_buffer->data()
                                           , frame_buffer->size()
                                           , true);
                auto samples = audio_format_helper(compose_frame.format()).samples_from_size(frame_buffer->size());
                if (m_audio_track.demix(stream_buffer.map()
                                        , samples))
                {
                    audio_frame_impl stream_frame(compose_frame.format()
                                                 , compose_frame.frame_id()
                                                 , compose_frame.timestamp());

                    stream_frame.smart_buffers().set_buffer(main_media_buffer_index
                                                            , std::move(stream_buffer));
                    message_frame_ref_impl message_frame(stream_frame);
                    return m_router.send_message(message_frame);
                }
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

        // i_parametrizable interface
    public:
        bool set_params(const i_property &params) override
        {
            return m_params.load(params);
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
        mutable mutex_t         m_safe_mutex;
        media_composer&         m_owner;

        composer_stream::map_t  m_streams;

        stream_id_t             m_stream_ids;

        friend class composer_stream;

    public:
        stream_manager(media_composer& owner)
            : m_owner(owner)
            , m_stream_ids(0)
        {

        }

        inline const composer_params_t& composer_params() const
        {
            return m_owner.m_composer_params;
        }

        i_media_converter::u_ptr_t create_converter(media_type_t media_type)
        {
            switch(media_type)
            {
                case media_type_t::audio:
                    return m_owner.create_audio_converter();
                break;
                case media_type_t::video:
                    return m_owner.create_video_converter();
                break;
                default:;
            }

            return nullptr;
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

        composer_stream::s_array_t active_streams(bool sort_by_order = false) const
        {
            composer_stream::s_array_t array;
            {
                shared_lock_t lock(m_safe_mutex);
                for (const auto& s : m_streams)
                {
                    if (auto stream = s.second.lock())
                    {
                        array.emplace_back(std::move(stream));
                    }
                }
            }

            if (sort_by_order)
            {
                static auto compare_handler = [](const composer_stream::s_ptr_t& lhs, const composer_stream::s_ptr_t& rhs)
                {
                    return lhs->compare(*rhs);
                };
                std::sort(array.begin(), array.end(), compare_handler);
            }

            return array;
        }

    private:

        void on_remove_stream(composer_stream* stream)
        {
            lock_t lock(m_safe_mutex);
            m_streams.erase(stream->stream_id());
        }

    };

    class video_frame_composer
    {
        i_layout_manager&       m_layout_manager;
        video_format_impl       m_format;
        image_frame_t           m_output_image;
        video_image_builder     m_image_builder;

        video_frame_ref_impl    m_output_frame;

        frame_id_t              m_frame_id;
        timestamp_t             m_timestamp;
        timestamp_t             m_start_time;

    public:

        video_frame_composer(i_layout_manager& layout_manager
                             , const i_video_format& video_format)
            : m_layout_manager(layout_manager)
            , m_format(video_format)
            , m_output_image(video_format.format_id()
                             , { video_format.width(), video_format.height() })
            , m_image_builder({}, nullptr)
            , m_output_frame(m_format)
            , m_frame_id(0)
            , m_timestamp(0)
            , m_start_time(0)
        {
            m_output_image.tune();
            m_image_builder.set_output_frame(&m_output_image);
            m_output_frame.smart_buffers().set_buffer(main_media_buffer_index
                                                      , smart_buffer(&m_output_image.image_data));
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

        inline const i_video_format& format() const
        {
            return m_format;
        }

        inline image_frame_t& output_image()
        {
            return m_output_image;
        }

        void compose_streams(composer_stream::s_array_t& streams)
        {
            if (!streams.empty())
            {
                auto layout = get_layout(streams);

                m_image_builder.blackout();

                std::size_t idx = 0;

                auto tp = mpl::core::utils::now();

                for (auto& s : streams)
                {
                    if (s->is_custom())
                    {
                        s->compose_video(&m_output_image);
                    }
                    else if (layout)
                    {
                        s->compose_video(&m_output_image
                                         , layout->get_rect(idx++));
                    }
                }

                auto dt = mpl::core::utils::now() - tp;
                auto sdt = mpl::core::utils::now();

                sdt = mpl::core::utils::now() - sdt;

                /*std::clog << "Compose time: " << durations::to_microseconds(dt)
                          << ", send time: " << durations::to_microseconds(sdt)
                          << ", total: " << durations::to_microseconds(dt + sdt) << std::endl;*/

                if (auto frame = next_frame())
                {
                    for (auto& s : streams)
                    {
                        message_frame_ref_impl message_frame(*frame);
                        s->feedback_frame(message_frame);
                    }
                }

            }
        }

        const i_video_frame* next_frame()
        {
            if (!m_output_image.is_empty())
            {
                auto now = mpl::core::utils::get_ticks();

                if (m_start_time == 0)
                {
                    m_start_time = now;
                }

                auto elapsed_time = now - m_start_time;

                m_output_frame.set_frame_id(m_frame_id);
                m_output_frame.set_timestamp(m_timestamp);

                m_frame_id++;


                //m_timestamp += video_sample_rate / m_format.frame_rate();

                m_timestamp = (elapsed_time * video_sample_rate) / durations::seconds(1);

                if (m_format.frame_rate())
                {
                    timestamp_t duration = video_sample_rate / m_format.frame_rate();
                    m_timestamp -= m_timestamp % (duration / 2);
                }

                return &m_output_frame;
            }

            return nullptr;
        }
    };

    class audio_frame_composer
    {
        audio_format_impl       m_format;
        smart_buffer            m_output_buffer;

        audio_frame_ref_impl    m_output_frame;

        frame_id_t              m_frame_id;
        timestamp_t             m_timestamp;
        timestamp_t             m_start_time;

    public:
        audio_frame_composer(const i_audio_format& audio_format
                             , std::size_t samples)
            : m_format(audio_format)
            , m_output_buffer(nullptr
                              , audio_format_helper(m_format).size_from_samples(samples))
            , m_output_frame(m_format)
            , m_frame_id(0)
            , m_timestamp(0)
            , m_start_time(0)
        {
            m_output_frame.smart_buffers().set_buffer(main_media_buffer_index
                                                      , smart_buffer(&m_output_buffer));
        }

        inline const i_audio_format& format() const
        {
            return m_format;
        }

        void compose_streams(composer_stream::s_array_t& streams)
        {
            if (!streams.empty())
            {
                if (auto audio_data = m_output_buffer.map())
                {
                    std::memset(audio_data
                                , 0
                                , m_output_buffer.size());

                    auto samples = audio_format_helper(m_output_frame.format())
                            .samples_from_size(m_output_buffer.size());

                    for (auto& s : streams)
                    {
                        s->mix_audio(audio_data
                                     , samples);
                    }

                    if (auto compose_frame = next_frame())
                    {
                        for (auto& s : streams)
                        {
                            s->compose_audio(*compose_frame);
                        }
                    }

                }
            }
        }

        const i_audio_frame* next_frame()
        {
            if (!m_output_buffer.is_empty())
            {
                auto now = mpl::core::utils::get_ticks();

                if (m_start_time == 0)
                {
                    m_start_time = now;
                }

                m_output_frame.set_frame_id(m_frame_id);
                m_output_frame.set_timestamp(m_timestamp);

                m_frame_id++;

                m_timestamp += audio_format_helper(m_format).samples_from_size(m_output_buffer.size());

                return &m_output_frame;
            }

            return nullptr;
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

    i_media_converter_factory&  m_converter_factory;
    composer_params_t           m_composer_params;
    stream_manager              m_stream_manager;

    audio_frame_composer        m_audio_composer;
    video_frame_composer        m_video_composer;

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
        : m_converter_factory(converter_factory)
        , m_composer_params(std::move(composer_params))
        , m_stream_manager(*this)
        , m_audio_composer(m_composer_params.audio_params.format
                           , m_composer_params.audio_params.frame_size())
        , m_video_composer(layout_manager
                           , m_composer_params.video_params.format)
        , m_started(false)
    {

    }

    ~media_composer()
    {
        media_composer::stop();
    }


    i_media_converter::u_ptr_t create_video_converter()
    {
        video_format_impl format(m_composer_params.video_params.format);
        format.set_width(0);
        format.set_height(0);

        if (auto params = format.get_params("format"))
        {
            return m_converter_factory.create_converter(*params);
        }

        return nullptr;
    }

    i_media_converter::u_ptr_t create_audio_converter()
    {
        if (auto params = m_composer_params.audio_params.format.get_params("format"))
        {
            return m_converter_factory.create_converter(*params);
        }

        return nullptr;
    }

    void video_compose_proc()
    {
        m_video_compose_delay.reset();
        std::size_t frames = 0;
        auto tp = mpl::core::utils::get_ticks();
        while(m_started)
        {
            timestamp_t video_delay = durations::second / m_video_composer.format().frame_rate();

            auto streams = m_stream_manager.active_streams(true);
            m_video_composer.compose_streams(streams);
            m_video_compose_delay.wait(video_delay);
            frames++;

            auto dt = mpl::core::utils::get_ticks() - tp;

            auto fps = durations::seconds(frames) / dt;


            // std::clog << "fps: " << fps << std::endl;
        }
    }

    void audio_compose_proc()
    {
        m_audio_compose_delay.reset();
        while(m_started)
        {
            auto streams = m_stream_manager.active_streams();
            m_audio_composer.compose_streams(streams);

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
