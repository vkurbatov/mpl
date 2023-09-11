#include "vnc_device_factory.h"


#include "core/message_router_impl.h"
#include "core/property_writer.h"
#include "core/message_event_impl.h"
#include "core/event_channel_state.h"
#include "core/time_utils.h"

#include "video_frame_impl.h"
#include "message_frame_impl.h"

#include "tools/base/sync_base.h"
#include "tools/vnc/vnc_device.h"

#include <thread>
#include <shared_mutex>
#include <atomic>

namespace mpl::media
{

namespace detail
{

video_format_id_t format_form_bpp(std::uint32_t bpp)
{
    switch(bpp)
    {
        case 8:
            return video_format_id_t::gray8;
        break;
        case 16:
            return video_format_id_t::bgr565;
        break;
        case 24:
            return video_format_id_t::bgr24;
        break;
        case 32:
            return video_format_id_t::bgr32;
        break;
        default:;
    }

    return video_format_id_t::undefined;
}

}

class vnc_device : public i_device
{
    using mutex_t = base::shared_spin_lock;
    using lock_t = std::lock_guard<mutex_t>;
    using shared_lock_t = std::shared_lock<mutex_t>;

    using u_ptr_t = std::unique_ptr<vnc_device>;

    struct device_params_t
    {
        device_type_t       device_type;
        std::string         host;
        std::uint16_t       port;
        std::string         password;
        std::uint32_t       fps;

        device_params_t(device_type_t device_type = device_type_t::vnc
                        , const std::string_view& host = {}
                        , std::uint16_t port = vnc::default_port
                        , const std::string_view& password = {}
                        , std::uint32_t fps = vnc::default_fps)
            : device_type(device_type)
            , host(host)
            , port(port)
            , password(password)
            , fps(fps)
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
            if (reader.get("device_type", device_type_t::vnc) == device_type_t::vnc)
            {
                return reader.get("host", host)
                        | reader.get("port", port)
                        | reader.get("password", password)
                        | reader.get("fps", fps);
            }

            return false;
        }

        bool save(i_property& params) const
        {
            property_writer writer(params);
            return writer.set("device_type", device_type_t::vnc)
                    && writer.set("host", host)
                    && writer.set("port", port)
                    && writer.set("password", password)
                    && writer.set("fps", fps);
        }

        bool is_valid() const
        {
            return device_type == device_type_t::vnc
                    && !host.empty()
                    && port != 0
                    && fps > 0;
        }

        vnc::vnc_config_t native_config()
        {
            return { host, password, port };
        }
    };

    device_params_t             m_device_params;
    message_router_impl         m_router;
    vnc::vnc_device             m_native_device;

    frame_id_t                  m_frame_counter;
    timestamp_t                 m_frame_timestamp;
    timestamp_t                 m_real_timestamp;

    timestamp_t                 m_start_time;

    std::thread                 m_thread;

    channel_state_t             m_state;
    std::atomic_bool            m_running;
    bool                        m_open;

public:

    static u_ptr_t create(const i_property &params)
    {
        device_params_t vnc_params(params);
        if (vnc_params.is_valid())
        {
            return std::make_unique<vnc_device>(std::move(vnc_params));
        }

        return nullptr;
    }

    vnc_device(device_params_t&& device_params)
        : m_device_params(std::move(device_params))
        , m_native_device(m_device_params.native_config())
        , m_frame_counter(0)
        , m_frame_timestamp(0)
        , m_real_timestamp(0)
        , m_state(channel_state_t::ready)
        , m_running(false)
        , m_open(false)
    {

    }

    ~vnc_device() override
    {
        close();
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
        if (!m_open)
        {
            m_open = true;
            m_running.store(true, std::memory_order_release);

            change_state(channel_state_t::opening);

            m_thread = std::thread([&]{ grabbing_thread(); });
            return true;
        }

        return false;
    }

    bool close()
    {
        if (m_open)
        {
            change_state(channel_state_t::closing);
            m_running.store(false, std::memory_order_release);
            m_open = false;

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
        m_real_timestamp = 0;
        m_start_time = mpl::core::utils::now();
    }

    timestamp_t elapsed_time() const
    {
        return mpl::core::utils::now() - m_start_time;
    }

    void process_timesatamp(std::uint32_t fps)
    {
        m_real_timestamp = (elapsed_time() * video_sample_rate) / durations::seconds(1);
        m_frame_timestamp = m_real_timestamp;
        if (fps != 0)
        {
            auto duration = video_sample_rate / fps;
            m_frame_timestamp -= m_frame_timestamp % (duration / 2);
        }
    }

    bool on_native_frame(vnc::frame_t&& frame)
    {
        video_format_impl video_format(detail::format_form_bpp(frame.bpp)
                                       , frame.frame_size.width
                                       , frame.frame_size.height);

        if (video_format.format_id() != video_format_id_t::undefined
                && !frame.frame_data.empty())
        {
            video_frame_impl video_frame(std::move(video_format)
                                         , m_frame_counter
                                         , m_frame_timestamp
                                         , i_video_frame::frame_type_t::undefined);

            video_frame.smart_buffers().set_buffer(media_buffer_index
                                                   , smart_buffer(std::move(frame.frame_data)));

            m_frame_counter++;
            process_timesatamp(m_device_params.fps);

            message_frame_ref_impl message_frame(video_frame);

            return m_router.send_message(message_frame);
        }

        return false;
    }

    void grabbing_thread()
    {
        change_state(channel_state_t::open);

        std::size_t error_counter = 0;

        std::uint32_t frame_time = 1000 / m_device_params.fps;

        while(is_running())
        {
            change_state(channel_state_t::connecting);
            if (m_native_device.open())
            {
                change_state(channel_state_t::connected);

                error_counter = 0;

                while (is_running()
                       && error_counter < 10)
                {
                    vnc::frame_t v4l2_frame;
                    switch(m_native_device.fetch_frame(v4l2_frame
                                                       , frame_time * 2))
                    {
                        case vnc::io_result_t::complete:
                        {
                            error_counter = 0;
                            on_native_frame(std::move(v4l2_frame));
                        }
                        break;
                        case vnc::io_result_t::timeout:
                            error_counter++;
                        break;
                        case vnc::io_result_t::error:
                        case vnc::io_result_t::not_ready:
                            error_counter = 11;
                        break;
                    }

                    if (is_running())
                    {
                        core::utils::sleep(durations::milliseconds(frame_time));
                    }
                }

                change_state(channel_state_t::disconnecting);
                m_native_device.close();
                change_state(channel_state_t::disconnected);
            }
        }
    }


    bool is_running() const
    {
        return m_running.load(std::memory_order_acquire);
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
        return m_open;
    }

    channel_state_t state() const override
    {
        return m_state;
    }

    // i_message_channel interface
public:
    i_message_sink *sink(std::size_t index) override
    {
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

    // i_device interface
public:
    device_type_t device_type() const override
    {
        return device_type_t::vnc;
    }
    // i_parametrizable interface
public:
    bool set_params(const i_property &params) override
    {
        bool result = false;

        auto device_params = m_device_params;

        if (device_params.load(params)
                && device_params.is_valid())
        {
            if (!m_native_device.is_opened()
                    && m_native_device.set_config(device_params.native_config()))
            {
                m_device_params = device_params;
                result = true;
            }
        }

        return result;
    }

    bool get_params(i_property &params) const override
    {
        return m_device_params.save(params);
    }
};

vnc_device_factory::u_ptr_t vnc_device_factory::create()
{
    return std::make_unique<vnc_device_factory>();
}

vnc_device_factory::vnc_device_factory()
{

}

i_device::u_ptr_t vnc_device_factory::create_device(const i_property &device_params)
{
    return vnc_device::create(device_params);
}

}
