#include "udp_transport_factory.h"
#include "udp_transport_params.h"

#include "utils/property_utils.h"
#include "i_udp_transport.h"

#include "net_utils.h"

#include "tools/io/net/udp_link.h"

namespace mpl::net
{


class udp_transport_impl final: public i_udp_transport
{
    udp_transport_params_t      m_udp_params;
    io::udp_link                m_link;
public:
    // i_channel interface
public:
    bool control(const channel_control_t &control) override
    {
        return false;
    }

    bool is_open() const override
    {
        return m_link.is_open();
    }

    channel_state_t state() const override
    {
        return get_channel_state(m_link.state());
    }

    // i_parametrizable interface
public:
    bool set_params(const i_property &params) override
    {
        if (!is_open())
        {
            return utils::property::deserialize(m_udp_params
                                                , params);
        }

        return false;
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
        return nullptr;
    }

    i_message_source *source(std::size_t index) override
    {
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
    bool set_local_endpoint(const socket_endpoint_t &endpoint) override
    {
        if (!is_open())
        {
            m_udp_params.local_endpoint = endpoint;
            return true;
        }

        return false;
    }

    bool set_remote_endpoint(const socket_endpoint_t &endpoint) override
    {
        m_udp_params.remote_endpoint = endpoint;
        return true;
    }

    const socket_endpoint_t &local_endpoint() const override
    {
        return m_udp_params.local_endpoint;
    }

    const socket_endpoint_t &remote_endpoint() const override
    {
        return m_udp_params.remote_endpoint;
    }
};


udp_transport_factory::u_ptr_t udp_transport_factory::create(io::io_core &io_core)
{
    return std::make_unique<udp_transport_factory>(io_core);
}

udp_transport_factory::udp_transport_factory(io::io_core &io_core)
    : m_io_core(io_core)
{

}

i_transport_channel::u_ptr_t udp_transport_factory::create_transport(const i_property &params)
{
    return nullptr;
}


}
