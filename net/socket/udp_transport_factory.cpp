#include "udp_transport_factory.h"
#include "udp_transport_params.h"

#include "i_udp_transport.h"
#include "i_socket_packet.h"

#include "socket_packet_impl.h"

#include "core/event_channel_state.h"

#include "utils/message_event_impl.h"
#include "utils/property_utils.h"
#include "utils/smart_buffer.h"
#include "utils/message_router_impl.h"
#include "utils/message_sink_impl.h"

#include "net/net_message_types.h"
#include "net/net_utils.h"
#include "net/net_engine_impl.h"

#include "tools/io/net/udp_link.h"
#include "tools/io/net/udp_link_config.h"
#include "tools/io/net/resolver.h"

namespace mpl::net
{


class udp_transport_impl final: public i_udp_transport
        , public i_message_sink
{
    udp_transport_params_t          m_udp_params;
    pt::io::udp_link                m_link;
    pt::io::resolver                m_resolver;

    message_router_impl             m_router;

    channel_state_t                 m_state;

public:

    using u_ptr_t = std::unique_ptr<udp_transport_impl>;

    static u_ptr_t create(const i_property& params
                          , pt::io::io_core& core)
    {
        udp_transport_params_t udp_params;
        if (utils::property::deserialize(udp_params
                                         , params))
        {
            return std::make_unique<udp_transport_impl>(std::move(udp_params)
                                                        , core);
        }

        return nullptr;
    }

    udp_transport_impl(udp_transport_params_t&& udp_params
                       , pt::io::io_core& core)
        : m_udp_params(std::move(udp_params))
        , m_link(core
                 , m_udp_params.options)
        , m_resolver(core)
        , m_state(channel_state_t::ready)
    {
        m_link.set_message_handler([&](auto&& ...args) { on_link_message(args...); });
        m_link.set_state_handler([&](auto&& ...args) { on_link_state(args...); });
    }

    ~udp_transport_impl()
    {
        m_resolver.reset();
        m_link.control(pt::io::link_control_id_t::close);
        m_link.set_message_handler(nullptr);
        m_link.set_state_handler(nullptr);
    }

public:

    void change_channel_state(channel_state_t new_state
                              , const std::string_view& reason = {})
    {
        if (m_state != new_state)
        {
            m_state = new_state;
            if (new_state == channel_state_t::open)
            {
                m_udp_params.local_endpoint.socket_address = m_link.local_endpoint();
            }
            m_router.send_message(message_event_impl(event_channel_state_t(new_state
                                                                            , reason))
                                  );
        }
    }

    bool internal_send_packet(const i_message_packet &packet)
    {
        if (m_link.is_open())
        {
            if (packet.subclass() == message_class_net)
            {
                auto& net_packet = static_cast<const i_net_packet&>(packet);
                if (net_packet.transport_id() == transport_id_t::udp)
                {
                    return internal_send_socket_packet(static_cast<const i_socket_packet&>(net_packet));
                }
            }
        }
        return false;
    }

    bool internal_send_socket_packet(const i_socket_packet& socket_packet)
    {
        if (socket_packet.size() > 0)
        {
            pt::io::message_t message(socket_packet.data()
                                  , socket_packet.size());

            if (socket_packet.address().is_valid())
            {
                return m_link.send_to(message
                                      , socket_packet.address());
            }
            else
            {
                return m_link.send(message);
            }
        }

        return false;
    }

    void on_link_message(const pt::io::message_t& message
                         , const pt::io::endpoint_t& endpoint)
    {
        if (endpoint.type == pt::io::endpoint_t::type_t::ip)
        {
            udp_endpoint_t socket_endpoint(static_cast<const socket_address_t&>(endpoint));
            smart_buffer packet_buffer(message.data()
                                       , message.size());

            udp_packet_impl socket_packet(std::move(packet_buffer)
                                          , socket_endpoint.socket_address);

            m_router.send_message(socket_packet);
        }
    }

    void on_link_state(pt::io::link_state_t link_state
                       , const std::string_view& reason)
    {
        change_channel_state(utils::get_channel_state(link_state)
                             , reason);
    }

    // i_channel interface
public:
    bool control(const channel_control_t &control) override
    {
        switch(control.control_id)
        {
            case channel_control_id_t::open:
            {
                m_link.set_local_endpoint(m_udp_params.local_endpoint.socket_address);
                m_link.set_remote_endpoint(m_udp_params.remote_endpoint.socket_address);
                return m_link.control(pt::io::link_control_id_t::open);
            }
            break;
            case channel_control_id_t::close:
                return m_link.control(pt::io::link_control_id_t::close);
            break;
            case channel_control_id_t::connect:
                return m_link.control(pt::io::link_control_id_t::start);
            break;
            case channel_control_id_t::shutdown:
                return m_link.control(pt::io::link_control_id_t::stop);
            break;
            default:;
        }

        return false;
    }

    bool is_open() const override
    {
        return m_link.is_open();
    }

    channel_state_t state() const override
    {
        return m_state;
    }

    // i_parametrizable interface
public:
    bool set_params(const i_property &params) override
    {
        return utils::property::deserialize(m_udp_params
                                            , params);
    }

    bool get_params(i_property &params) const override
    {
        return utils::property::serialize(m_udp_params
                                          , params);
    }

    // i_message_channel interface
public:
    i_message_sink *sink(std::size_t index) override
    {
        if (index == 0)
        {
            return this;
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

    // i_transport_channel interface
public:
    transport_id_t transport_id() const override
    {
        return transport_id_t::udp;
    }

    // i_socket_transport interface
public:
    bool set_local_endpoint(const udp_endpoint_t &endpoint) override
    {
        if (!is_open())
        {
            m_udp_params.local_endpoint = endpoint;
            return true;
        }

        return false;
    }

    bool set_remote_endpoint(const udp_endpoint_t &endpoint) override
    {
        m_udp_params.remote_endpoint = endpoint;
        return true;
    }

    udp_endpoint_t local_endpoint() const override
    {
        return m_udp_params.local_endpoint;
    }

    udp_endpoint_t remote_endpoint() const override
    {
        return m_udp_params.remote_endpoint;
    }

    // i_message_sink interface
public:
    bool send_message(const i_message &message) override
    {
        switch(message.category())
        {
            case message_category_t::packet:
                return internal_send_packet(static_cast<const i_message_packet&>(message));
            break;
            default:;
        }

        return false;
    }
};


udp_transport_factory::u_ptr_t udp_transport_factory::create(net_engine_impl &engine)
{
    return std::make_unique<udp_transport_factory>(engine);
}

udp_transport_factory::udp_transport_factory(net_engine_impl &engine)
    : m_engine(engine)
{

}

i_transport_channel::u_ptr_t udp_transport_factory::create_transport(const i_property &params)
{
    return udp_transport_impl::create(params
                                      , m_engine.io_core());
}


}
