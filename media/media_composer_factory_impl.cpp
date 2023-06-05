#include "media_composer_factory_impl.h"
#include "i_media_converter_factory.h"
#include "i_message_frame.h"
#include "i_audio_frame.h"
#include "i_video_frame.h"
#include "i_audio_format.h"
#include "i_video_format.h"

#include "video_frame_impl.h"

#include "core/i_buffer_collection.h"
#include "core/message_router_impl.h"
#include "core/message_sink_impl.h"
#include "core/time_utils.h"

#include "media_types.h"
#include "image_frame.h"
#include "draw_options.h"
#include "video_image_builder.h"
#include "message_frame_impl.h"

#include "tools/base/sync_base.h"

#include <shared_mutex>
#include <thread>
#include <queue>
#include <unordered_map>
#include <cmath>

namespace mpl::media
{

using layout_t = std::vector<relative_frame_rect_t>;
using layout_array_t = std::vector<layout_t>;

constexpr std::size_t default_max_frame_queue = 5;

namespace detail
{

layout_t generate_mosaic_layout(std::size_t streams)
{
    layout_t layouts;

    if (streams < 2)
    {
        layouts.emplace_back(0.0, 0.0, 1.0, 1.0);
    }
    else if (streams == 2)
    {
        layouts.emplace_back(0.0, 0.25, 0.5, 0.5);
        layouts.emplace_back(0.5, 0.25, 0.5, 0.5);
    }
    else
    {
        auto col_count = static_cast<std::int32_t>(std::sqrt(streams) + 0.999);
        auto last_col_count = streams % col_count;
        auto row_count = static_cast<std::int32_t>(streams) / col_count + static_cast<std::int32_t>(last_col_count != 0);

        double l_width = 1.0 / col_count;
        double l_heigth = 1.0 / row_count;

        for (auto row = 0; row < row_count; row ++)
        {
            auto l_y = static_cast<double>(row) * l_heigth;

            auto x_offset = 0.0;

            if ((row + 1 == row_count) && (last_col_count != 0))
            {
                x_offset = (static_cast<double>(col_count - last_col_count) * l_width) / 2.0;
            }

            for (auto col = 0; col < col_count && layouts.size() < streams; col++)
            {
                auto l_x = x_offset + static_cast<double>(col) * l_width;
                layouts.emplace_back(l_x
                                    , l_y
                                    , l_width
                                    , l_heigth);
            }
        }
    }

    return layouts;
}

const layout_t& get_mosaic_layouts(std::size_t streams)
{
    static layout_array_t static_layouts;
    static base::spin_lock safe_mutex;

    if (static_layouts.size() <= streams)
    {
        std::lock_guard lock(safe_mutex);
        while (static_layouts.size() <= (streams + 5))
        {
            static_layouts.emplace_back(generate_mosaic_layout(static_layouts.size()));
        }
    }

    return static_layouts[streams];
}

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

    class stream_manager;

    class composer_stream : public i_media_stream
            , i_message_sink
            , std::enable_shared_from_this<composer_stream>
    {

        class media_track
        {
            using frame_queue_t = std::queue<i_media_frame::s_ptr_t>;
            mutable mutex_t             m_safe_mutex;

            message_sink_impl           m_message_sink;
            i_media_converter::u_ptr_t  m_media_converter;

            frame_queue_t               m_frame_queue;
            i_media_frame::s_ptr_t      m_last_frame;

        public:

            media_track(i_media_converter::u_ptr_t&& media_converter)
                : m_message_sink([&](const auto& message_frame) { return on_converter_message(message_frame); })
                , m_media_converter(std::move(media_converter))
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
                        m_frame_queue.pop();
                    }
                }

                return m_last_frame;
            }

            void clear()
            {
                lock_t lock(m_safe_mutex);
                m_frame_queue = {};
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


        stream_manager&         m_manager;
        message_router_impl     m_router;


        media_track             m_audio_track;
        media_track             m_video_track;

        stream_id_t             m_stream_id;

    public:

        using s_ptr_t = std::shared_ptr<composer_stream>;
        using w_ptr_t = std::weak_ptr<composer_stream>;
        using s_array_t = std::vector<s_ptr_t>;
        using map_t = std::unordered_map<stream_id_t, w_ptr_t>;

        static s_ptr_t create(stream_manager& manager
                              , const i_property& stream_params)
        {
            return std::make_shared<composer_stream>(manager
                                                     , stream_params);
        }

        composer_stream(stream_manager& manager
                        , const i_property& stream_params)
            : m_manager(manager)
            , m_audio_track(manager.create_converter(media_type_t::audio))
            , m_video_track(manager.create_converter(media_type_t::video))
            , m_stream_id(manager.next_stream_id())
        {

        }

        ~composer_stream()
        {
            m_manager.on_remove_stream(this);
        }

        const i_video_frame* pop_video_frame()
        {
            if (auto media_frame = m_video_track.pop_frame())
            {
                if (media_frame->media_type() == media_type_t::video)
                {
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

        // i_parametrizable interface
    public:
        bool set_params(const i_property &params) override
        {
            return false;
        }

        bool get_params(i_property &params) const override
        {
            return false;
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
            shared_lock_t lock(m_safe_mutex);
            for (const auto& s : m_streams)
            {
                if (auto stream = s.second.lock())
                {
                    array.emplace_back(std::move(stream));
                }
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


    public:

        video_frame_composer(const i_video_format& video_format)
            : m_format(video_format)
            , m_output_image(video_format.format_id()
                             , { video_format.width(), video_format.height() })
            , m_image_builder({}, nullptr)
            , m_output_frame(m_format)
            , m_frame_id(0)
            , m_timestamp(0)
        {
            m_output_image.tune();
            m_image_builder.set_output_frame(&m_output_image);
        }

        const i_video_format& format() const
        {
            return m_format;
        }

        bool blackout()
        {
            return m_image_builder.blackout();
        }

        bool push_frame(const i_video_frame& frame
                        , const draw_options_t& draw_options)
        {
            auto image = detail::create_image(frame);
            if (image.is_valid())
            {
                return m_image_builder.draw_image_frame(image
                                                        , draw_options);
            }

            return false;
        }

        const i_video_frame* compose_frame()
        {
            if (!m_output_image.is_empty())
            {
                m_output_frame.set_frame_id(m_frame_id);
                m_output_frame.set_timestamp(m_timestamp);
                m_output_frame.smart_buffers().set_buffer(main_media_buffer_index
                                                          , m_output_image.image_data);

                m_frame_id++;
                m_timestamp += video_sample_rate / m_format.frame_rate();

                return &m_output_frame;
            }

            return nullptr;
        }
    };

    i_media_converter_factory&  m_converter_factory;
    stream_manager              m_stream_manager;
    video_frame_composer        m_video_composer;
    message_router_impl         m_router;

    std::thread                 m_compose_thread;

    bool                        m_started;

public:
    using u_ptr_t = std::unique_ptr<media_composer>;

    static u_ptr_t create(i_media_converter_factory& converter_factory
                          , const i_property &params)
    {
        return std::make_unique<media_composer>(converter_factory
                                                , params);
    }

    media_composer(i_media_converter_factory& converter_factory
                   , const i_property &params)
        : m_converter_factory(converter_factory)
        , m_stream_manager(*this)
        , m_video_composer(video_format_impl(video_format_id_t::bgr24
                                             , 1280, 720, 30))
        , m_started(false)
    {

    }

    ~media_composer()
    {
        media_composer::stop();
    }

    i_media_converter::u_ptr_t create_video_converter()
    {
        video_format_impl format(m_video_composer.format());
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
            auto layout = detail::get_mosaic_layouts(streams.size());
            m_video_composer.blackout();
            std::size_t idx = 0;
            for (auto& s : streams)
            {
                if (auto video_frame = s->pop_video_frame())
                {
                    draw_options_t options(layout[idx]);
                    m_video_composer.push_frame(*video_frame
                                                , options);
                }

                idx++;
            }

            if (auto frame = m_video_composer.compose_frame())
            {
                message_frame_ref_impl message_frame(*frame);
                m_router.send_message(message_frame);
            }
        }
    }

    void compose_proc()
    {
        while(m_started)
        {
            auto streams = m_stream_manager.active_streams();
            compose_video(streams);
            mpl::core::utils::sleep(durations::microseconds(1000000 / m_video_composer.format().frame_rate()));
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
        return false;
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
            m_started = true;
            m_compose_thread = std::thread([&]{ compose_proc(); });

            return true;
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

media_composer_factory_impl::u_ptr_t media_composer_factory_impl::create(i_media_converter_factory &media_converter_factory)
{
    return std::make_unique<media_composer_factory_impl>(media_converter_factory);
}

media_composer_factory_impl::media_composer_factory_impl(i_media_converter_factory &media_converter_factory)
    : m_media_converter_factory(media_converter_factory)
{

}

i_media_composer::u_ptr_t media_composer_factory_impl::create_composer(const i_property &params)
{
    return media_composer::create(m_media_converter_factory
                                  , params);
}


}
