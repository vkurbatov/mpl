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

    class composer_stream : public i_media_stream
            , i_message_sink
            , std::enable_shared_from_this<composer_stream>
    {

        class audio_composer
        {

            composer_stream&    m_owner;
            audio_mixer         m_input_mixer;
            audio_mixer         m_output_mixer;
            audio_level         m_audio_level;

            audio_composer(composer_stream& owner
                           , const i_audio_format& audio_format
                           , timestamp_t durations)
                : m_owner(owner)
                , m_input_mixer(audio_format
                                , audio_format_helper(audio_format).duration_form_samples(durations))
                , m_output_mixer(audio_format
                                 , audio_format_helper(audio_format).duration_form_samples(durations))
                , m_audio_level(audio_level::config_t{})
            {

            }


            bool push_data(const void* sample_data
                           , std::size_t samples
                           , double level)
            {
                if (m_input_mixer.overrun() > 0)
                {
                    m_input_mixer.reset();
                    m_output_mixer.reset();
                }

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
                if (m_input_mixer.copy_data(m_output_mixer
                                            , samples) == samples)
                {
                    return m_input_mixer.pop_data(data
                                                , samples
                                                , audio_mixer::mix_method_t::mix) == samples;
                }

                return false;
            }

            bool demix(void* data
                       , std::size_t samples)
            {
                return m_output_mixer.pop_data(data
                                                , samples
                                                , audio_mixer::mix_method_t::demix) == samples;
            }


            void reset()
            {
                m_input_mixer.reset();
                m_output_mixer.reset();
            }

        };

        class media_track
        {
            using frame_queue_t = std::queue<i_media_frame::s_ptr_t>;
            mutable mutex_t             m_safe_mutex;

            message_sink_impl           m_message_sink;
            i_media_converter::u_ptr_t  m_media_converter;

            frame_queue_t               m_frame_queue;
            i_media_frame::s_ptr_t      m_last_frame;
            timestamp_t                 m_last_frame_time;

        public:

            media_track(i_media_converter::u_ptr_t&& media_converter)
                : m_message_sink([&](const auto& message_frame) { return on_converter_message(message_frame); })
                , m_media_converter(std::move(media_converter))
                , m_last_frame_time(0)
            {
                if (m_media_converter)
                {
                    m_media_converter->set_sink(&m_message_sink);
                }
            }

            ~media_track()
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

        image_frame_t           m_user_image;

        media_track             m_audio_track;
        media_track             m_video_track;

        video_image_builder     m_image_builder;

        stream_id_t             m_stream_id;

    public:

        using s_ptr_t = std::shared_ptr<composer_stream>;
        using w_ptr_t = std::weak_ptr<composer_stream>;
        using s_array_t = std::vector<s_ptr_t>;
        using map_t = std::unordered_map<stream_id_t, w_ptr_t>;

        static s_ptr_t create(stream_manager& manager
                              , const i_property& stream_params
                              , image_frame_t* output_image = nullptr)
        {
            return std::make_shared<composer_stream>(manager
                                                     , stream_params
                                                     , output_image);
        }

        composer_stream(stream_manager& manager
                        , const i_property& stream_params
                        , image_frame_t* output_image = nullptr)
            : m_params(stream_params)
            , m_manager(manager)
            , m_audio_track(manager.create_converter(media_type_t::audio))
            , m_video_track(manager.create_converter(media_type_t::video))
            , m_image_builder({}, output_image)
            , m_stream_id(manager.next_stream_id())
        {
            if (!m_params.user_image_path.empty())
            {
                m_user_image.load(m_params.user_image_path
                                  , output_image->format_id);
            }
        }

        ~composer_stream()
        {
            m_manager.on_remove_stream(this);
        }

        void set_output_image(image_frame_t* image)
        {
            m_image_builder.set_output_frame(image);
        }

        const i_video_frame* pop_video_frame()
        {
            if (m_stream_id == 45)
            {
                return nullptr;
            }

            if (auto media_frame = m_video_track.pop_frame())
            {
                if (media_frame->media_type() == media_type_t::video)
                {
                    // 5-15 usec delay
                    return static_cast<const i_video_frame*>(media_frame.get());
                }
            }

            return nullptr;
        }


        const i_audio_frame* pop_audio_frame()
        {
            if (auto media_frame = m_audio_track.pop_frame())
            {
                if (media_frame->media_type() == media_type_t::audio)
                {
                    return static_cast<const i_audio_frame*>(media_frame.get());
                }
            }

            return nullptr;
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

        bool compose_image(const relative_frame_rect_t& target_rect = {})
        {
            draw_options_t options = m_params.draw_options;
            if (!target_rect.is_null())
            {
                options.target_rect = target_rect;
            }

            if (!options.target_rect.size.is_null())
            {
                if (auto video_frame = pop_video_frame())
                {
                    auto image = detail::create_image(*video_frame);
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
                                                      , stream_params
                                                      , &m_owner.m_video_composer.output_image()))
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
        video_format_impl       m_format;
        image_frame_t           m_output_image;
        video_image_builder     m_image_builder;

        video_frame_ref_impl    m_output_frame;

        frame_id_t              m_frame_id;
        timestamp_t             m_timestamp;
        timestamp_t             m_start_time;

    public:

        video_frame_composer(const i_video_format& video_format)
            : m_format(video_format)
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

        inline const i_video_format& format() const
        {
            return m_format;
        }

        inline bool blackout()
        {
            return m_image_builder.blackout();
        }

        inline image_frame_t& output_image()
        {
            return m_output_image;
        }

        const i_video_frame* compose_frame()
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

    struct composer_params_t
    {
        audio_format_impl   audio_format;
        video_format_impl   video_format;
        std::string         user_img_path;

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
                if (!audio_format.set_params(*audio_format_params))
                {
                    return false;
                }

                result = true;
            }

            if (auto video_format_params = reader["video_params.format"])
            {
                if (!video_format.set_params(*video_format_params))
                {
                    return false;
                }

                result = true;
            }

            result |= reader.get("user_img", user_img_path);

            return result;
        }

        bool save(i_property& params) const
        {
            property_writer writer(params);

            return writer.set("audio_params.format", audio_format)
                    & writer.set("video_params.format", video_format)
                    & writer.set("user_img", user_img_path, {});

        }

        inline bool has_video() const
        {
            if (video_format.is_valid()
                    && video_format.frame_rate() > 0)
            {
                switch(video_format.format_id())
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

        inline bool has_audio() const
        {
            return false;
        }

        inline bool is_valid() const
        {
            return has_video()
                    || has_audio();
        }
    };

    i_media_converter_factory&  m_converter_factory;
    i_layout_manager&           m_layout_manager;
    composer_params_t           m_composer_params;
    stream_manager              m_stream_manager;
    video_frame_composer        m_video_composer;
    message_router_impl         m_router;

    adaptive_delay              m_video_compose_delay;

    std::thread                 m_compose_thread;

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
        , m_layout_manager(layout_manager)
        , m_composer_params(std::move(composer_params))
        , m_stream_manager(*this)
        , m_video_composer(composer_params.video_format)
        , m_started(false)
    {

    }

    ~media_composer()
    {
        media_composer::stop();
    }

    const i_layout* get_layout(const composer_stream::s_array_t& streams)
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
        video_format_impl format(m_composer_params.video_format);
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
        return nullptr;
    }

    void compose_video(composer_stream::s_array_t& streams)
    {
        if (!streams.empty())
        {
            auto layout = get_layout(streams);

            m_video_composer.blackout();

            std::size_t idx = 0;

            auto tp = mpl::core::utils::now();

            for (auto& s : streams)
            {
                if (s->is_custom())
                {
                    s->compose_image();
                }
                else if (layout)
                {
                    s->compose_image(layout->get_rect(idx++));
                }

            }

            auto dt = mpl::core::utils::now() - tp;
            auto sdt = mpl::core::utils::now();

            if (auto frame = m_video_composer.compose_frame())
            {
                message_frame_ref_impl message_frame(*frame);
                m_router.send_message(message_frame);
            }

            sdt = mpl::core::utils::now() - sdt;
            std::clog << "Compose time: " << durations::to_microseconds(dt)
                      << ", send time: " << durations::to_microseconds(sdt)
                      << ", total: " << durations::to_microseconds(dt + sdt) << std::endl;
        }
    }

    void compose_proc()
    {
        m_video_compose_delay.reset();
        std::size_t frames = 0;
        auto tp = mpl::core::utils::get_ticks();
        while(m_started)
        {
            timestamp_t video_delay = durations::second / m_video_composer.format().frame_rate();

            auto streams = m_stream_manager.active_streams(true);
            compose_video(streams);

            m_video_compose_delay.wait(video_delay);
            frames++;

            auto dt = mpl::core::utils::get_ticks() - tp;

            auto fps = durations::seconds(frames) / dt;


            std::clog << "fps: " << fps << std::endl;
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
                m_compose_thread = std::thread([&]{ compose_proc(); });

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
            if (m_compose_thread.joinable())
            {
                m_compose_thread.join();
            }

            return true;
        }

        return false;
    }

    bool is_started() const override
    {
        return m_started;
    }

    i_message_source *source() override
    {
        return &m_router;
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
