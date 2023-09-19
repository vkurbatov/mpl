#include "visca_device_factory.h"

#include "core/enum_converter_defs.h"
#include "core/enum_serialize_defs.h"

#include "core/message_router_impl.h"
#include "core/message_sink_impl.h"
#include "core/property_writer.h"
#include "core/message_event_impl.h"
#include "core/event_channel_state.h"
#include "core/time_utils.h"

#include "media_command_message_impl.h"
#include "command_camera_control.h"

#include "tools/io/serial/serial_link.h"
#include "tools/io/serial/serial_link_config.h"
#include "tools/io/serial/serial_endpoint.h"
#include "tools/io/io_core.h"
#include "tools/visca/i_visca_channel.h"
#include "tools/visca/visca_control.h"

#include <thread>
#include <shared_mutex>
#include <condition_variable>
#include <atomic>

namespace mpl::core::utils
{

declare_enum_converter_begin(io::serial_parity_t)
    declare_pair(io::serial_parity_t, none),
    declare_pair(io::serial_parity_t, odd),
    declare_pair(io::serial_parity_t, even)
declare_enum_converter_end(io::serial_parity_t)

declare_enum_converter_begin(io::serial_stop_bits_t)
    declare_pair(io::serial_stop_bits_t, one),
    declare_pair(io::serial_stop_bits_t, onepointfive),
    declare_pair(io::serial_stop_bits_t, two)
declare_enum_converter_end(io::serial_stop_bits_t)

declare_enum_converter_begin(io::serial_flow_control_t)
    declare_pair(io::serial_flow_control_t, none),
    declare_pair(io::serial_flow_control_t, software),
    declare_pair(io::serial_flow_control_t, hardware)
declare_enum_converter_end(io::serial_flow_control_t)

}

namespace mpl
{

declare_enum_serializer(io::serial_parity_t)
declare_enum_serializer(io::serial_stop_bits_t)
declare_enum_serializer(io::serial_flow_control_t)

}


namespace mpl::media
{

class visca_device : public i_device
{
    using raw_array_t = std::vector<std::uint8_t>;
    using mutex_t = std::mutex;
    using cond_t = std::condition_variable;


    class visca_channel : public visca::i_visca_channel
    {
        mutable std::mutex      m_sync_mutex;
        cond_t                  m_cond;
        visca_device&           m_owner;
        io::serial_link         m_serial_link;
        visca::packet_data_t    m_recv_data;
    public:
        visca_channel(visca_device& owner
                      , io::io_core& io_core
                      , const io::serial_link_config_t& serial_config)
            : m_owner(owner)
            , m_serial_link(io_core
                            , serial_config)
        {
            m_serial_link.set_state_handler([&](auto&& ...args) { on_link_state(args...); });
            m_serial_link.set_message_handler([&](auto&& ...args) { on_message(args...); });
        }

        ~visca_channel()
        {
            m_serial_link.control(io::link_control_id_t::close);
        }

        void on_link_state(io::link_state_t new_state
                           , const std::string_view& reason)
        {
            switch(new_state)
            {
                case io::link_state_t::ready:
                    m_owner.change_state(channel_state_t::ready, reason);
                break;
                case io::link_state_t::opening:
                    m_owner.change_state(channel_state_t::opening, reason);
                break;
                case io::link_state_t::open:
                    m_owner.change_state(channel_state_t::open, reason);
                break;
                case io::link_state_t::connecting:
                    m_owner.change_state(channel_state_t::connecting, reason);
                break;
                case io::link_state_t::connected:
                    m_owner.change_state(channel_state_t::connected, reason);
                break;
                case io::link_state_t::disconnecting:
                    m_owner.change_state(channel_state_t::disconnecting, reason);
                break;
                case io::link_state_t::disconnected:
                    m_owner.change_state(channel_state_t::disconnected, reason);
                break;
                case io::link_state_t::closing:
                    m_owner.change_state(channel_state_t::closing, reason);
                break;
                case io::link_state_t::closed:
                    m_owner.change_state(channel_state_t::closed, reason);
                break;
                case io::link_state_t::failed:
                    m_owner.change_state(channel_state_t::failed, reason);
                break;
                default:;
            }
        }

        void on_message(const io::message_t& message
                        , const io::endpoint_t& endpoint)
        {
            std::lock_guard lock(m_sync_mutex);
            if (auto data = static_cast<const std::uint8_t*>(message.data()))
            {
                m_recv_data.insert(m_recv_data.end()
                                   , data
                                   , data + message.size());
                m_cond.notify_all();
            }
        }

        // i_visca_channel interface
    public:
        bool open() override
        {
            return m_serial_link.control(io::link_control_id_t::open)
                    && m_serial_link.control(io::link_control_id_t::start);
        }

        bool close() override
        {
            return m_serial_link.control(io::link_control_id_t::close);
        }

        std::size_t write(const void *data, std::size_t size) override
        {
            io::message_t message(data
                                  , size);
            if (m_serial_link.send(message))
            {
                return size;
            }

            return 0;
        }

        std::size_t read(visca::packet_data_t &data, uint32_t timeout) override
        {
            auto handler = [&]{ return !m_recv_data.empty(); };
            std::unique_lock lock(m_sync_mutex);

            m_cond.wait_for(lock
                            , std::chrono::milliseconds(timeout)
                            , handler);
            if (!m_recv_data.empty())
            {
                data = std::move(m_recv_data);
                return true;
            }

            return false;
        }

        bool flush() override
        {
            std::lock_guard lock(m_sync_mutex);
            m_recv_data.clear();
            return true;
        }

        bool is_open() const override
        {
            return m_serial_link.is_open();
        }
    };

    struct device_params_t
    {
        visca::visca_config_t       visca_config;
        io::serial_link_config_t    serial_config;
        io::serial_endpoint_t       serial_endpoint;

        device_params_t(const visca::visca_config_t& visca_config = {}
                        , const io::serial_link_config_t& serial_config = {}
                        , const io::serial_endpoint_t& serial_endpoint = {})
        {

        }

        device_params_t(const i_property& params)
        {
            load(params);
        }

        bool load(const i_property& params)
        {
            property_reader reader(params);
            if (reader.get("device_type", device_type_t::visca) == device_type_t::visca)
            {
                return reader.get("visca.reply_timeout_ms", visca_config.reply_timeout)
                        | reader.get("visca.pan_speed",visca_config.pan_speed)
                        | reader.get("visca.tilt_speed",visca_config.tilt_speed)
                        | reader.get("serial.baud_rate", serial_config.baud_rate)
                        | reader.get("serial.char_size", serial_config.char_size)
                        | reader.get("serial.parity", serial_config.parity)
                        | reader.get("serial.stop_bits", serial_config.stop_bits)
                        | reader.get("serial.flow_control", serial_config.flow_control)
                        | reader.get("serial.port_name", serial_endpoint.port_name);
            }

            return false;
        }

        bool save(i_property& params) const
        {
            property_writer writer(params);
            return writer.set("device_type", device_type_t::apm)
                        && writer.set("visca.reply_timeout_ms", visca_config.reply_timeout)
                        && writer.set("visca.pan_speed",visca_config.pan_speed)
                        && writer.set("visca.tilt_speed",visca_config.tilt_speed)
                        && writer.set("serial.baud_rate", serial_config.baud_rate)
                        && writer.set("serial.char_size", serial_config.char_size)
                        && writer.set("serial.parity", serial_config.parity)
                        && writer.set("serial.stop_bits", serial_config.stop_bits)
                        && writer.set("serial.flow_control", serial_config.flow_control)
                        && writer.set("serial.port_name", serial_endpoint.port_name);
        }

        inline bool is_valid() const
        {
            return serial_config.is_valid()
                    && serial_endpoint.is_valid();
        }
    };

    device_params_t             m_device_params;
    visca_channel               m_visca_channel;
    visca::visca_control        m_visca_control;

    message_sink_impl           m_sink;
    message_router_impl         m_router;

    channel_state_t             m_state;

public:

    using u_ptr_t = std::unique_ptr<visca_device>;

    static io::io_core& get_gore()
    {
        auto& core = io::io_core::get_instance();
        if (!core.is_running())
        {
            core.run();
        }

        return core;
    }

    static u_ptr_t create(const i_property& device_params)
    {
        device_params_t visca_params(device_params);
        if (visca_params.is_valid())
        {
            return std::make_unique<visca_device>(std::move(visca_params));
        }
        return nullptr;
    }

    visca_device(device_params_t&& device_params)
        : m_device_params(std::move(device_params))
        , m_visca_channel(*this
                          , get_gore()
                          , m_device_params.serial_config)
        , m_visca_control(m_device_params.visca_config
                          , &m_visca_channel)
        , m_sink([&](auto&& ...args) { return on_message(args...); })
        , m_state(channel_state_t::ready)
    {

    }

    ~visca_device()
    {

    }

    void change_state(channel_state_t new_state
                      , const std::string_view& reason)
    {
        if (m_state != new_state)
        {
            m_state = new_state;
            m_router.send_message(message_event_impl<event_channel_state_t>({new_state, reason}));
        }
    }

    bool on_message(const i_message& message)
    {
        switch(message.category())
        {
            case message_category_t::command:
            {
                auto& command_message = static_cast<const i_message_command&>(message);
                switch(command_message.command().command_id)
                {
                    case command_camera_control_t::id:
                    {
                        return on_camera_control(static_cast<const command_camera_control_t&>(command_message.command()));
                    }
                    break;
                    default:;
                }
            }
            break;
            default:;
        }

        return false;
    }

    bool on_camera_control(const command_camera_control_t& camera_control)
    {
        if (is_open())
        {

            /*
            lock_t lock(m_command_mutex);
            m_commands.push(camera_control);
            m_command_signal.notify_all();*/

            return true;
        }
        return false;
    }

    // i_channel interface
public:
    bool control(const channel_control_t &control) override
    {
        switch(control.control_id)
        {
            case channel_control_id_t::open:
                return m_visca_channel.open();
            break;
            case channel_control_id_t::close:
                return m_visca_channel.close();
            break;
            default:;
        }

        return false;
    }

    bool is_open() const override
    {
        return m_visca_channel.is_open();
    }

    channel_state_t state() const override
    {
        return m_state;
    }

    // i_message_channel interface
public:
    i_message_sink *sink(std::size_t index) override
    {
        if (index == 0)
        {
            return &m_sink;
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

    // i_device interface
public:
    device_type_t device_type() const override
    {
        return device_type_t::visca;
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
            if (!m_visca_channel.is_open())
            {
                m_device_params = device_params;
                // m_visca_channel.set_config(device_params.wap_config);
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

i_device::u_ptr_t visca_device_factory::create_device(const i_property &device_params)
{
    return visca_device::create(device_params);
}


}
