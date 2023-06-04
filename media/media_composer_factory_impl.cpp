#include "media_composer_factory_impl.h"
#include "i_media_converter_factory.h"
#include "i_message_frame.h"
#include "i_audio_frame.h"
#include "i_video_frame.h"

#include "core/message_router_impl.h"
#include "core/message_sink_impl.h"


#include "tools/base/sync_base.h"
#include <shared_mutex>
#include <thread>
#include <queue>
#include <map>

namespace mpl::media
{

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

            composer_stream&            m_owner;
            message_sink_impl           m_message_sink;
            i_media_converter::u_ptr_t  m_media_converter;

            frame_queue_t               m_frame_queue;
            i_message_frame::s_ptr_t    m_last_frame;

        public:

            media_track(composer_stream& owner
                        , i_media_converter::u_ptr_t&& media_converter)
                : m_owner(owner)
                , m_message_sink([&](const auto& message_frame) { return on_converter_message(message_frame); })
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

        private:

            bool on_converter_frame(const i_message_frame& message_frame)
            {
                lock_t lock(m_safe_mutex);
                return false;
            }

            bool on_converter_message(const i_message& message)
            {
                switch(message.category())
                {
                    case message_category_t::frame:
                        return on_converter_frame(static_cast<const i_message_frame&>(message));
                    break;
                }

                /*if (auto clone_frame = i_message_frame::s_ptr_t(message_frame.clone()))
                {
                    m_frame_queue.push(std::move(clone_frame));
                }*/

                return false;
            }
        };

        stream_manager&         m_manager;
        message_router_impl     m_router;

        stream_id_t             m_stream_id;

    public:

        using s_ptr_t = std::shared_ptr<composer_stream>;
        using w_ptr_t = std::weak_ptr<composer_stream>;
        using map_t = std::map<stream_id_t, w_ptr_t>;

        static s_ptr_t create(stream_manager& manager
                              , const i_property& stream_params)
        {
            return std::make_shared<composer_stream>(manager
                                                     , stream_params);
        }

        composer_stream(stream_manager& manager
                        , const i_property& stream_params)
            : m_manager(manager)
            , m_stream_id(manager.next_stream_id())
        {

        }

        ~composer_stream()
        {
            m_manager.on_remove_stream(this);
        }

        bool push_frame(const i_message_frame& message_frame)
        {
            switch(message_frame.frame().media_type())
            {
                case media_type_t::audio:

                break;
                case media_type_t::video:

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

    private:

        void on_remove_stream(composer_stream* stream)
        {
            lock_t lock(m_safe_mutex);
            m_streams.erase(stream->stream_id());
        }

    };

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

    }

    void compose_proc()
    {
        while(m_started)
        {

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
        return nullptr;
    }

    i_media_stream::s_ptr_t get_stream(stream_id_t stream_id) const override
    {
        return nullptr;
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
