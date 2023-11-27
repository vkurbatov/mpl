#include "visca_device_factory.h"

#include "core/event_channel_state.h"

#include "utils/enum_converter_defs.h"
#include "utils/enum_serialize_defs.h"
#include "utils/message_router_impl.h"
#include "utils/message_sink_impl.h"
#include "utils/property_writer.h"
#include "utils/message_event_impl.h"
#include "utils/time_utils.h"
#include "utils/pointer_utils.h"

#include "media/media_types.h"
#include "media/media_module_types.h"
#include "media/command_camera_control.h"

#include "utils/message_command_impl.h"

/*
#include "tools/io/serial/serial_link.h"
#include "tools/io/serial/serial_link_config.h"
#include "tools/io/serial/serial_endpoint.h"
#include "tools/io/io_core.h"*/

#include "net/i_transport_factory.h"
#include "net/serial/i_serial_transport.h"
#include "net/serial/serial_packet_impl.h"
#include "net/serial/serial_transport_params.h"
#include "net/net_module_types.h"


#include "tools/visca/i_visca_channel.h"
#include "tools/visca/visca_control.h"

#include <thread>
#include <shared_mutex>
#include <condition_variable>
#include <atomic>

namespace mpl::media
{

namespace detail
{

struct control_info_t
{
    using array_t = std::vector<control_info_t>;

    std::string     id;
    std::string     name;
    std::int16_t    value;
    std::int16_t    min;
    std::int16_t    max;
    std::int16_t    step;

    control_info_t(const std::string_view& id = {}
                   , const std::string_view& name = {}
                   , std::int16_t value = 0
                   , std::int16_t min = 0
                   , std::int16_t max = 0
                   , std::int16_t step = 0)
        : id(id)
        , name(name)
        , value(value)
        , min(min)
        , max(max)
        , step(step)
    {

    }

    bool load(const i_property& params)
    {
        property_reader reader(params);
        if (reader.get("id", id))
        {
            reader.get("value", value);
            reader.get("name", name);
            reader.get("min", min);
            reader.get("max", max);
            reader.get("step", step);
            return true;
        }

        return false;
    }

    bool save(i_property& params) const
    {
        property_writer writer(params);
        return writer.set("id", id)
                && writer.set("name", name)
                && writer.set("value", value)
                && writer.set("min", min)
                && writer.set("max", max)
                && writer.set("step", step);
    }

    bool is_valid() const
    {
        return !id.empty();
    }
};

static const control_info_t::array_t visca_control_table =
{
    { "pan_absolute", "Pan Absolute", 0, pt::visca::visca_pan_min, pt::visca::visca_pan_max, 1 },
    { "tilt_absolute", "Tilt Absolute", 0, pt::visca::visca_tilt_min, pt::visca::visca_tilt_max, 1 },
    { "zoom_absolute", "Zoom Absolute", 0, pt::visca::visca_zoom_min, pt::visca::visca_zoom_max, 1 }
};

}

class visca_device_impl : public i_device
{
    struct device_params_t;

    using raw_array_t = std::vector<std::uint8_t>;
    using mutex_t = std::mutex;
    using lock_t = std::lock_guard<mutex_t>;
    using cond_t = std::condition_variable;


    class visca_channel : public pt::visca::i_visca_channel
    {
        mutable std::mutex                  m_sync_mutex;
        cond_t                              m_cond;
        visca_device_impl&                  m_owner;
        message_sink_impl                   m_serial_sink;
        net::i_serial_transport::u_ptr_t    m_serial_transport;
        pt::visca::packet_data_t            m_recv_data;
        pt::visca::visca_control            m_visca_control;

    public:

        visca_channel(visca_device_impl& owner
                      , net::i_serial_transport::u_ptr_t serial_transport
                      , const device_params_t& device_params)
            : m_owner(owner)
            , m_serial_sink([&](auto&& ...args) { return on_message(args...); })
            , m_serial_transport(std::move(serial_transport))
            , m_visca_control(device_params.visca_config
                              , this)
        {
            m_serial_transport->source(0)->add_sink(&m_serial_sink);
        }

        ~visca_channel()
        {
            m_serial_transport->control(channel_control_t::close());
            m_serial_transport->source(0)->remove_sink(&m_serial_sink);
        }


        bool on_serial_packet(const net::i_serial_packet& serial_packet)
        {
            std::lock_guard lock(m_sync_mutex);
            if (auto data = static_cast<const std::uint8_t*>(serial_packet.data()))
            {
                m_recv_data.insert(m_recv_data.end()
                                   , data
                                   , data + serial_packet.size());
                m_cond.notify_all();
                return true;
            }

            return false;
        }

        bool on_message(const i_message& message)
        {
            if (message.category() == message_category_t::packet
                    && message.module_id() == net::net_module_id)
            {
                auto& net_packet = static_cast<const net::i_net_packet&>(message);
                if (net_packet.transport_id() == net::transport_id_t::serial)
                {
                    return on_serial_packet(static_cast<const net::i_serial_packet&>(net_packet));
                }
            }

            return true;
        }

        bool execute_command(detail::control_info_t& control
                             , bool is_set)
        {
            if (control.id == "pan_absolute")
            {
                return is_set
                        ? m_visca_control.set_pan(control.value)
                        : m_visca_control.get_pan(control.value);
            }
            else if (control.id == "tilt_absolute")
            {
                return is_set
                        ? m_visca_control.set_tilt(control.value)
                        : m_visca_control.get_tilt(control.value);
            }
            else if (control.id == "zoom_absolute")
            {
                return is_set
                        ? m_visca_control.set_zoom(control.value)
                        : m_visca_control.get_zoom(control.value);
            }

            return false;
        }

        bool execute_command(i_property& output_params)
        {
            detail::control_info_t control_info;
            property_writer writer(output_params);

            if (control_info.load(output_params))
            {
                auto is_set = writer.has_property("value");
                if (execute_command(control_info
                                    , is_set))
                {
                    if (!is_set)
                    {
                        writer.set("value", control_info.value);
                    }

                    return true;
                }
                else
                {
                    writer.remove("value");
                }
            }

            return false;
        }

        inline std::size_t execute_commands(i_property& output_params)
        {
            std::size_t result = 0;

            if (output_params.property_type() == property_type_t::array)
            {
                const auto& ctrls = static_cast<const i_property_array&>(output_params).get_value();
                for (const auto& c : ctrls)
                {
                    if (c != nullptr)
                    {
                        if (execute_command(*c))
                        {
                            result++;
                        }
                    }
                }
            }

            return result;
        }

        inline bool execute_control_command(command_camera_control_t& camera_control)
        {
            if (camera_control.state == command_camera_control_t::state_t::request)
            {
                if (camera_control.commands == nullptr)
                {
                    camera_control.commands = get_controls();
                    return camera_control.commands != nullptr;
                }
                else
                {
                    if (execute_commands(*camera_control.commands) > 0)
                    {
                        return true;
                    }
                }
            }

            return false;
        }

        inline i_property::u_ptr_t get_controls() const
        {
            if (auto ctrls = property_helper::create_array())
            {
                auto& control_array = static_cast<i_property_array&>(*ctrls).get_value();
                for (auto& c : detail::visca_control_table)
                {
                    if (auto ctrl = property_helper::create_object())
                    {
                        if (c.save(*ctrl))
                        {
                            control_array.emplace_back(std::move(ctrl));
                        }
                    }
                }

                return ctrls;
            }
            return nullptr;
        }

        // i_visca_channel interface
    public:
        bool open() override
        {
            return m_serial_transport->control(channel_control_t::open())
                    && m_serial_transport->control(channel_control_t::connect());
        }

        bool close() override
        {
            return m_serial_transport->control(channel_control_t::close());
        }

        std::size_t write(const void *data, std::size_t size) override
        {
            net::serial_packet_impl serial_packet(smart_buffer(data
                                                               , size)
                                                  , m_owner.m_device_params.serial_params.serial_endpoint.port_name);
            if (auto sink = m_serial_transport->sink(0))
            {
                if (sink->send_message(serial_packet))
                {
                    return size;
                }
            }

            return 0;
        }

        std::size_t read(pt::visca::packet_data_t &data, uint32_t timeout) override
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
            return m_serial_transport->is_open();
        }
    };

    struct device_params_t
    {
        pt::visca::visca_config_t       visca_config;
        net::serial_transport_params_t  serial_params;

        device_params_t(const pt::visca::visca_config_t& visca_config = {}
                        , const net::serial_transport_params_t& serial_params = {})
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
                        | reader.get("serial", serial_params);
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
                        && writer.set("serial", serial_params);
        }

        inline bool is_valid() const
        {
            return serial_params.is_valid();
        }
    };

    using commands_queue_t = std::queue<command_camera_control_t>;

    mutable mutex_t             m_command_mutex;
    cond_t                      m_command_signal;

    device_params_t             m_device_params;
    visca_channel               m_visca_channel;

    message_sink_impl           m_sink;
    message_router_impl         m_router;

    commands_queue_t            m_commands;

    std::thread                 m_thread;

    channel_state_t             m_state;
    std::atomic_bool            m_running;
    bool                        m_open;

public:

    using u_ptr_t = std::unique_ptr<visca_device_impl>;

    static net::i_serial_transport::u_ptr_t create_serial_transport(net::i_transport_factory& serial_factory
                                                                    , const net::serial_transport_params_t& serial_params)
    {
        if (auto serial_property = utils::property::serialize(serial_params))
        {
            return utils::static_pointer_cast<net::i_serial_transport>(serial_factory.create_transport(*serial_property));
        }

        return nullptr;
    }

    static u_ptr_t create(net::i_transport_factory& serial_factory
                          , const i_property& device_params)
    {
        device_params_t visca_params(device_params);
        if (visca_params.is_valid())
        {
            if (auto serial_transport = create_serial_transport(serial_factory
                                                                     , visca_params.serial_params))
            {
                return std::make_unique<visca_device_impl>(std::move(serial_transport)
                                                           , std::move(visca_params));
            }
        }
        return nullptr;
    }

    visca_device_impl(net::i_serial_transport::u_ptr_t&& serial_transport
                    , device_params_t&& device_params)
        : m_device_params(std::move(device_params))
        , m_visca_channel(*this
                          , std::move(serial_transport)
                          , m_device_params)
        , m_sink([&](auto&& ...args) { return on_message(args...); })
        , m_state(channel_state_t::ready)
        , m_running(false)
        , m_open(false)
    {

    }

    ~visca_device_impl()
    {
        close();
    }

    inline void change_state(channel_state_t new_state
                      , const std::string_view& reason = {})
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

    inline bool on_camera_control(const command_camera_control_t& camera_control)
    {
        if (is_open())
        {
            lock_t lock(m_command_mutex);
            m_commands.push(camera_control);
            m_command_signal.notify_all();

            return true;
        }
        return false;
    }

    inline bool fetch_control_command(command_camera_control_t& camera_control)
    {
        lock_t lock(m_command_mutex);
        if (!m_commands.empty())
        {
            camera_control = std::move(m_commands.front());
            m_commands.pop();
            return true;
        }

        return false;
    }

    inline bool is_running() const
    {
        return m_running.load(std::memory_order_acquire);
    }

    void control_proc()
    {
        std::mutex signal_mutex;
        std::unique_lock signal_lock(signal_mutex);
        change_state(channel_state_t::open);

        while(is_running())
        {
            change_state(channel_state_t::connecting);
            if (m_visca_channel.open())
            {
                change_state(channel_state_t::connected);

                while(is_running())
                {
                    command_camera_control_t camera_control;
                    if (fetch_control_command(camera_control))
                    {
                        if (m_visca_channel.execute_control_command(camera_control))
                        {
                            camera_control.state = command_camera_control_t::state_t::success;
                        }
                        else
                        {
                            camera_control.state = command_camera_control_t::state_t::failed;
                        }
                        m_router.send_message(message_command_impl<command_camera_control_t, media_module_id>(camera_control));
                        continue;
                    }
                    m_command_signal.wait_for(signal_lock, std::chrono::seconds(1));
                }

                change_state(channel_state_t::disconnecting);
                m_visca_channel.close();
                change_state(channel_state_t::disconnected);
            }

            m_command_signal.wait_for(signal_lock, std::chrono::milliseconds(100));
        }
    }

    bool open()
    {
        if (!m_open)
        {
            m_open = true;
            m_running.store(true, std::memory_order_release);

            change_state(channel_state_t::opening);

            m_thread = std::thread([&]{ control_proc(); });
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
            m_command_signal.notify_all();

            if (m_thread.joinable())
            {
                m_thread.join();
            }

            change_state(channel_state_t::closed);
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
            if (!m_open)
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

visca_device_factory::u_ptr_t visca_device_factory::create(net::i_transport_factory& transport_factory)
{
    return std::make_unique<visca_device_factory>(transport_factory);
}

visca_device_factory::visca_device_factory(net::i_transport_factory& transport_factory)
    : m_serial_factory(transport_factory)
{

}

i_device::u_ptr_t visca_device_factory::create_device(const i_property &device_params)
{
    return visca_device_impl::create(m_serial_factory
                                , device_params);
}


}