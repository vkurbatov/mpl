#include "socket_packet_builder_impl.h"
#include "socket_packet_impl.h"
#include "net/tls/tls_packet_impl.h"
#include "net/stun/stun_packet_impl.h"

namespace mpl::net
{

socket_packet_builder_impl::u_ptr_t socket_packet_builder_impl::create(transport_id_t transport_id
                                                                       , const socket_address_t &socket_address
                                                                       , const void *packet_data
                                                                       , std::size_t packet_size)
{
    switch(transport_id)
    {
        case transport_id_t::udp:
        case transport_id_t::tcp:
        case transport_id_t::ice:
        case transport_id_t::tls:
            return std::make_unique<socket_packet_builder_impl>(transport_id
                                                             , socket_address
                                                             , packet_data
                                                             , packet_size);
        break;
        default:;
    }

    return nullptr;
}

socket_packet_builder_impl::socket_packet_builder_impl(transport_id_t transport_id
                                                       , const socket_address_t &socket_address
                                                       , const void *packet_data
                                                       , std::size_t packet_size)
    : m_transport_id(transport_id)
    , m_socket_address(socket_address)
    , m_packet_buffer(packet_data
                      , packet_size
                      , true)

{

}

transport_id_t socket_packet_builder_impl::transport_id() const
{
    return m_transport_id;
}

void socket_packet_builder_impl::set_packet_data(const void *packet_data
                                                 , std::size_t packet_size)
{
    m_packet_buffer.assign(packet_data
                           , packet_size
                           , true);
}

const i_data_object &socket_packet_builder_impl::packet_data() const
{
    return m_packet_buffer;
}

i_net_packet::u_ptr_t socket_packet_builder_impl::build_packet()
{
    switch(m_transport_id)
    {
        case transport_id_t::udp:
            return udp_packet_impl::create(m_packet_buffer.fork()
                                           , m_socket_address);
        break;
        case transport_id_t::tcp:
            return tcp_packet_impl::create(m_packet_buffer.fork()
                                           , m_socket_address);
        break;
        case transport_id_t::ice:
            return stun_packet_impl::create(m_packet_buffer.fork()
                                           , m_socket_address);
        break;
        case transport_id_t::tls:
            return tls_packet_impl::create(m_packet_buffer.fork()
                                           , m_socket_address);
        break;
    }

    return nullptr;
}

void socket_packet_builder_impl::set_address(const socket_address_t &socket_address)
{
    m_socket_address = socket_address;
}

const socket_address_t &socket_packet_builder_impl::address() const
{
    return m_socket_address;
}



}
