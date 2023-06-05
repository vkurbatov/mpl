#include "media_composer_factory_impl.h"
#include "i_media_converter_factory.h"
#include "i_message_frame.h"
#include "i_audio_frame.h"
#include "i_video_frame.h"

#include "core/message_router_impl.h"
#include "core/message_sink_impl.h"
#include "core/time_utils.h"

#include "tools/base/sync_base.h"
#include <shared_mutex>
#include <thread>
#include <queue>
#include <unordered_map>

namespace mpl::media
{

constexpr std::size_t default_max_frame_queue = 5;

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
            using frame_queue_t = std::queue<i_message_frame::s_ptr_t>;
            mutable mutex_t             m_safe_mutex;

            message_sink_impl           m_message_sink;
            i_media_converter::u_ptr_t  m_media_converter;

            frame_queue_t               m_frame_queue;
            i_message_frame::s_ptr_t    m_last_frame;

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

                return on_converter_frame(message_frame);
            }

            i_message_frame::s_ptr_t pop_frame()
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


            bool on_converter_frame(const i_message_frame& message_frame)
            {
                if (auto clone_frame = std::static_pointer_cast<i_message_frame>(i_message::s_ptr_t(message_frame.clone())))
                {
                    lock_t lock(m_safe_mutex);
                    m_frame_queue.emplace(std::move(clone_frame));

                    while (m_frame_queue.size() > default_max_frame_queue)
                    {
                        m_frame_queue.pop();
                    }
                }
                return false;
            }


            bool on_converter_message(const i_message& message)
            {
                switch(message.category())
                {
                    case message_category_t::frame:
                        return on_converter_frame(static_cast<const i_message_frame&>(message));
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
            , m_audio_track(nullptr)
            , m_video_track(nullptr)
            , m_stream_id(manager.next_stream_id())
        {

        }

        ~composer_stream()
        {
            m_manager.on_remove_stream(this);
        }

        const i_video_frame* pop_video_frame()
        {
            if (auto message_frame = m_video_track.pop_frame())
            {
                if (message_frame->frame().media_type() == media_type_t::video)
                {
                    return static_cast<const i_video_frame*>(&message_frame->frame());
                }
            }

            return nullptr;
        }

        const i_audio_frame* pop_audio_frame()
        {
            if (auto message_frame = m_audio_track.pop_frame())
            {
                if (message_frame->frame().media_type() == media_type_t::audio)
                {
                    return static_cast<const i_audio_frame*>(&message_frame->frame());
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
        composer_stream::map_t  m_streams;

        stream_id_t             m_stream_ids;

        friend class composer_stream;

    public:
        stream_manager()
            : m_stream_ids(0)
        {

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

    stream_manager          m_stream_manager;
    message_router_impl     m_router;

    bool                    m_started;

public:
    using u_ptr_t = std::unique_ptr<media_composer>;

    static u_ptr_t create(const i_property &params)
    {
        return std::make_unique<media_composer>(params);
    }

    media_composer(const i_property &params)
        : m_started(false)
    {

    }

    ~media_composer()
    {
        media_composer::stop();
    }

    void compose_video(composer_stream::s_array_t& streams)
    {
        for (auto& s : streams)
        {
            if (auto video_frame = s->pop_video_frame())
            {

            }
        }
    }

    void compose_proc()
    {
        while(m_started)
        {
            auto streams = m_stream_manager.active_streams();
            compose_video(streams);
            mpl::core::utils::sleep(durations::milliseconds(50));
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
        }

        return false;
    }

    bool stop() override
    {
        if (m_started)
        {
            m_started = false;

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
    return media_composer::create(params);
}


}
