#include "libav_output_device_factory.h"

#include "core/message_router_impl.h"
#include "core/message_sink_impl.h"
#include "core/property_writer.h"
#include "core/message_event_impl.h"
#include "core/event_channel_state.h"
#include "core/time_utils.h"

#include "core/option_helper.h"


#include "media_option_types.h"
#include "audio_frame_impl.h"
#include "video_frame_impl.h"
#include "message_frame_impl.h"

#include "tools/base/sync_base.h"
#include "tools/ffmpeg/libav_stream_publisher.h"

#include <shared_mutex>
#include <thread>
#include <atomic>
#include <queue>

namespace mpl::media
{

constexpr std::size_t default_max_queue_size = 1000;

using stream_list_t = std::vector<i_media_format::u_ptr_t>;
using stream_map_t = std::unordered_map<std::int32_t, i_media_format::u_ptr_t>;

namespace detail
{

    template<typename Format>
    bool check_and_tune_format(Format& format
                               , std::int32_t force_idx)
    {
        if (format.is_valid())
        {
            option_writer writer(format.options());

            if (!writer.has_type<std::int32_t>(opt_fmt_stream_id))
            {
                writer.set<std::int32_t>(opt_fmt_stream_id
                                         , force_idx);
            }
        }

        return false;
    }

    bool deserialize_stream_list(const i_property& property
                                 , stream_list_t& stream_list)
    {
        bool result = false;
        if (property.property_type() == property_type_t::array)
        {
            for (const auto& f : static_cast<const i_property_array&>(property).get_value())
            {
                if (f != nullptr)
                {
                    switch(property_reader(*f).get("media_type", media_type_t::undefined))
                    {
                        case media_type_t::audio:
                        {
                            if (auto p = audio_format_impl::create(*f))
                            {
                                if (check_and_tune_format(*p
                                                          , stream_list.size()))
                                {
                                    result = true;
                                    stream_list.emplace_back(std::move(p));
                                }
                            }
                        }
                        break;
                        case media_type_t::video:
                        {
                            if (auto p = video_format_impl::create(*f))
                            {
                                if (check_and_tune_format(*p
                                                          , stream_list.size()))
                                {
                                    result = true;
                                    stream_list.emplace_back(std::move(p));
                                }
                            }
                        }
                        break;
                        default:;
                    }
                }
            }

        }

        return result;
    }

    i_property::u_ptr_t serialize_stream_list(const stream_list_t& stream_list)
    {
        i_property::array_t streams;

        for (const auto& s : stream_list)
        {
            if (s != nullptr)
            {
                switch(s->media_type())
                {
                    case media_type_t::audio:
                    {
                        const audio_format_impl& audio_format = static_cast<const audio_format_impl&>(*s);
                        if (auto p = property_helper::create_tree())
                        {
                            if (audio_format.get_params(*p))
                            {
                                streams.emplace_back(std::move(p));
                            }
                        }
                    }
                    break;
                    case media_type_t::video:
                    {
                        const video_format_impl& video_format = static_cast<const video_format_impl&>(*s);
                        if (auto p = property_helper::create_tree())
                        {
                            if (video_format.get_params(*p))
                            {
                                streams.emplace_back(std::move(p));
                            }
                        }
                    }
                    break;
                    default:;
                }
            }
        }

        if (!streams.empty())
        {
            return property_helper::create_array(std::move(streams));
        }

        return nullptr;
    }

}

/*
class libav_device_wrapper
{

    using state_handler_t = std::function<void(channel_state_t)>;
    ffmpeg::libav_stream_publisher  m_native_device;
    std::string                     m_url;
    state_handler_t                 m_state_handler;

    stream_list_t                   m_streams;

    libav_device_wrapper(const std::string& url
                         , const stream_list_t& streams
                         , state_handler_t&& state_handler)
        : m_url(url)
        , m_streams(streams)
        , m_state_handler(std::move(state_handler))
    {

    }

    bool push_frame(const i_media_frame& frame)
    {
        return false;
    }
};
*/

class libav_output_device : public i_device
{
    using thread_t = std::thread;
    using mutex_t = base::shared_spin_lock;
    using lock_t = std::lock_guard<mutex_t>;
    using shared_lock_t = std::shared_lock<mutex_t>;
    using libav_frame_queue_t = std::queue<ffmpeg::frame_t>;

    struct device_params_t
    {
        device_type_t   device_type = device_type_t::libav_out;
        std::string     url;
        stream_list_t   streams;

        device_params_t(device_type_t device_type = device_type_t::libav_out
                , const std::string_view& url = {}
                , stream_list_t&& streams = {})
            : device_type(device_type)
            , url(url)
            , streams(std::move(streams))
        {

        }

        device_params_t(const i_property& params)
            : device_params_t()
        {
            load(params);
        }


        bool load(const i_property& params)
        {
            property_reader reader(params);
            if (reader.get("url", url))
            {
                if (auto s  = reader["streams"])
                {
                    streams.clear();
                    return detail::deserialize_stream_list(*s
                                                           , streams);
                }
            }

            return false;
        }

        bool save(i_property& params) const
        {
            property_writer writer(params);
            if (writer.set("url", url))
            {
                if (auto s = detail::serialize_stream_list(streams))
                {
                    return writer.set("streams", s);
                }
            }

            return false;
        }

        ffmpeg::stream_info_list_t native_streams() const
        {
            ffmpeg::stream_info_list_t stream_list;
            for (const auto& s : streams)
            {
                ffmpeg::stream_info_t stream_info;
                if (s != nullptr
                        && core::utils::convert(*s
                                                , stream_info))
                {
                    stream_list.emplace_back(std::move(stream_info));
                }
            }

            return native_streams();
        }

        bool is_valid() const
        {
            return device_type == device_type_t::libav_out
                    && !url.empty()
                    && !streams.empty();
        }
    };

    class stream_output
    {

        libav_output_device&        m_owner;
        i_media_format::u_ptr_t     m_format;
        frame_id_t                  m_frame_counter;
        timestamp_t                 m_frame_timestamp;

        stream_output(libav_output_device& owner
                      , i_media_format::u_ptr_t&& format)
            : m_owner(owner)
            , m_format(std::move(format))
        {

        }
    };

    class frame_manager
    {
        mutable mutex_t             m_safe_mutex;

        libav_frame_queue_t         m_frame_queue;
        std::size_t                 m_max_queue_size;
    public:
        frame_manager(std::size_t max_queue_size = default_max_queue_size)
            : m_max_queue_size(max_queue_size)
        {

        }

        ~frame_manager()
        {
            clear();
        }

        void push_frame(ffmpeg::frame_t&& frame)
        {
            lock_t lock(m_safe_mutex);
            m_frame_queue.emplace(std::move(frame));
            while(m_frame_queue.size() > m_max_queue_size)
            {
                m_frame_queue.pop();
            }
        }

        bool fetch_frame(ffmpeg::frame_t& frame)
        {
            lock_t lock(m_safe_mutex);
            if (!m_frame_queue.empty())
            {
                frame = std::move(m_frame_queue.back());
                m_frame_queue.pop();
                return true;
            }

            return false;
        }

        libav_frame_queue_t fetch_frames()
        {
            lock_t lock(m_safe_mutex);
            return std::move(m_frame_queue);
        }

        void clear()
        {
            lock_t lock(m_safe_mutex);
            m_frame_queue = {};
        }
    };


    mutable mutex_t                 m_safe_mutex;
    thread_t                        m_thread;

    device_params_t                 m_device_params;
    message_sink_impl               m_message_sink;
    message_router_impl             m_router;

    frame_id_t                      m_frame_counter;
    timestamp_t                     m_frame_timestamp;

    timestamp_t                     m_start_time;

    frame_manager                   m_frame_manager;

    channel_state_t                 m_state;
    std::atomic_bool                m_open;

public:
    using u_ptr_t = std::unique_ptr<libav_output_device>;
    using s_ptr_t = std::shared_ptr<libav_output_device>;

    libav_output_device(device_params_t&& device_params)
        : m_device_params(std::move(device_params))
        , m_message_sink([&](const auto& message) { return on_message(message); } )
    {

    }

    ~libav_output_device()
    {
        // close();
    }

    void change_state(channel_state_t new_state
                      , const std::string_view& reason = {})
    {
        if (m_state != new_state)
        {
            m_state = new_state;
            m_router.send_message(message_event_impl<event_channel_state_t>({new_state, reason}));
        }
    }

    bool open()
    {
        bool expected = false;
        if (m_open.compare_exchange_strong(expected
                                           , true
                                           , std::memory_order_acquire))
        {
            reset();

            change_state(channel_state_t::opening);

            m_thread = std::thread([&]{ main_proc(); });

            m_open.store(false
                         , std::memory_order_release);
            change_state(channel_state_t::failed);
        }

        return false;
    }

    bool close()
    {
        if (m_open.load(std::memory_order_acquire))
        {
            change_state(channel_state_t::closing);

            m_open.store(false
                         , std::memory_order_release);

            if (m_thread.joinable())
            {
                m_thread.join();
            }

            change_state(channel_state_t::closed);

            return true;
        }

        return false;
    }


    void reset()
    {
        m_frame_counter = 0;
        m_frame_timestamp = 0;
        m_start_time = mpl::core::utils::now();
    }

    bool on_message_frame(const i_media_frame& media_frame)
    {
        if (m_open)
        {
            switch(media_frame.media_type())
            {
                case media_type_t::audio:
                {

                }
                break;
                case media_type_t::video:
                {

                }
                break;
                default:;
            }
        }
        return false;
    }

    bool on_message(const i_message& message)
    {
        switch(message.category())
        {
            case message_category_t::frame:
                return on_message_frame(static_cast<const i_message_frame&>(message).frame());
            break;
            default:;
        }

        return false;
    }

    void main_proc()
    {
        change_state(channel_state_t::open);

        while(libav_output_device::is_open())
        {
            ffmpeg::libav_stream_publisher native_publisher;
            change_state(channel_state_t::connecting);
            if (native_publisher.open(m_device_params.url
                                      , m_device_params.native_streams()))
            {
                change_state(channel_state_t::connected);
                while(libav_output_device::is_open())
                {
                    ffmpeg::frame_t libav_frame;
                    while(m_frame_manager.fetch_frame(libav_frame)
                          && libav_output_device::is_open())
                    {
                        if (!native_publisher.push_frame(libav_frame))
                        {
                            break;
                        }
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }
                change_state(channel_state_t::disconnecting);
            }
            change_state(channel_state_t::disconnected);
            native_publisher.close();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        change_state(channel_state_t::closed);
    }

    // i_channel interface
public:
    bool control(const channel_control_t &control) override
    {
        switch(control.control_id)
        {
            case channel_control_id_t::open:
                return open();
            break;
            case channel_control_id_t::close:
                return close();
            break;
            default:;
        }

        return false;
    }

    bool is_open() const override
    {
        return m_open.load(std::memory_order_acquire);
    }
    channel_state_t state() const override
    {
        return m_state;
    }

    // i_message_channel interface
public:
    i_message_sink *sink() override
    {
        return &m_message_sink;
    }
    i_message_source *source() override
    {
        return &m_router;
    }

    // i_device interface
public:
    device_type_t device_type() const override
    {
        return device_type_t::libav_out;
    }
};

libav_output_device_factory::libav_output_device_factory()
{

}

i_device::u_ptr_t libav_output_device_factory::create_device(const i_property &device_params)
{
    return nullptr;
}

}
