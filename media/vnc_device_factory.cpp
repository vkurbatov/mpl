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
                    | writer.set("host", host)
                    | writer.set("port", port)
                    | writer.set("password", password)
                    | writer.set("fps", fps);
        }

        bool is_valid() const
        {
            return device_type == device_type_t::vnc
                    && !host.empty()
                    && port != 0
                    && fps > 0;
        }

        vnc::vnc_server_config_t native_config()
        {
            return { host, password, port };
        }
    };

    device_params_t             m_device_params;
    message_router_impl         m_router;
    vnc::vnc_device             m_native_device;
    //v4l2_wrapper                m_wrapped_device;

    frame_id_t                  m_frame_counter;
    timestamp_t                 m_frame_timestamp;
    timestamp_t                 m_real_timestamp;

    timestamp_t                 m_start_time;

    channel_state_t             m_state;
    std::atomic_bool            m_running;
    bool                        m_open;

public:

    static u_ptr_t create(const i_property &params)
    {
        device_params_t vnc_params(params);
        if (vnc_params.is_valid())
        {
            return std::make_unique<vnc_device>(vnc_params);
        }

        return nullptr;
    }

    vnc_device(const device_params_t& device_params)
        : m_device_params(device_params)
        , m_native_device([&](auto&& frame) { return on_native_frame(std::move(frame)); }
                         , { m_device_params.fps })
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
            change_state(channel_state_t::opening);

            if (m_native_device.open(m_device_params.native_config()))
            {
                change_state(channel_state_t::open);
                change_state(channel_state_t::connecting);
                change_state(channel_state_t::connected);
                return true;
            }

            change_state(channel_state_t::failed);
        }

        return false;
    }

    bool close()
    {
        if (m_open)
        {
            change_state(channel_state_t::closing);
            m_open = false;

            m_native_device.close();

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
                                       , frame.frame_size.height
                                       , frame.fps);

        if (video_format.format_id() != video_format_id_t::undefined
                && !frame.frame_data.empty())
        {
            video_frame_impl video_frame(std::move(video_format)
                                         , m_frame_counter
                                         , m_frame_timestamp
                                         , i_video_frame::frame_type_t::undefined);

            video_frame.smart_buffers().set_buffer(main_media_buffer_index
                                                   , smart_buffer(std::move(frame.frame_data)));

            m_frame_counter++;
            process_timesatamp(frame.fps);

            message_frame_ref_impl message_frame(video_frame);

            return m_router.send_message(message_frame);
        }

        return false;
    }

    bool set_params(const i_property& input_params)
    {
        bool result = false;
        auto device_params = m_device_params;
        if (device_params.load(input_params)
                && device_params.is_valid())
        {
            if (!m_native_device.is_opened())
            {
                m_device_params = device_params;
                m_native_device.set_config({ device_params.fps });
                result = true;
            }
        }

        return result;
    }

    bool get_params(i_property& output_params)
    {
        if (m_device_params.save(output_params))
        {
            return true;
        }

        return false;
    }

    bool internal_configure(const i_property* input_params
                            , i_property* output_params)
    {
        bool result = false;

        if (input_params != nullptr)
        {
            result = set_params(*input_params);
        }

        if (output_params != nullptr)
        {
            result = get_params(*output_params);
        }

        return result;
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
            case channel_control_id_t::configure:
                return internal_configure(control.input_params
                                          , control.output_params);
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
    i_message_sink *sink() override
    {
        return nullptr;
    }

    i_message_source *source() override
    {
        return &m_router;
    }

    // i_device interface
public:
    device_type_t device_type() const override
    {
        return device_type_t::vnc;
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