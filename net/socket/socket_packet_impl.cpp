#include "socket_packet_impl.h"
//#include "net_types.h"

namespace mpl::net
{

socket_packet_impl::u_ptr_t socket_packet_impl::create(const smart_buffer &packet_buffer
                                                       , const socket_endpoint_t &endpoint)
{
    return std::make_unique<socket_packet_impl>(packet_buffer
                                                , endpoint);
}

socket_packet_impl::u_ptr_t socket_packet_impl::create(smart_buffer &&packet_buffer
                                                       , const socket_endpoint_t &endpoint)
{
    return std::make_unique<socket_packet_impl>(std::move(packet_buffer)
                                                , endpoint);
}

socket_packet_impl::socket_packet_impl(const smart_buffer &packet_buffer
                                       , const socket_endpoint_t &endpoint)
    : smart_buffer_container(packet_buffer)
    , m_socket_endpoint(endpoint)
{

}

socket_packet_impl::socket_packet_impl(smart_buffer &&packet_buffer
                                       , const socket_endpoint_t &endpoint)
    : smart_buffer_container(std::move(packet_buffer))
    , m_socket_endpoint(endpoint)
{

}


void socket_packet_impl::set_endpoint(const socket_endpoint_t &endpoint)
{
    m_socket_endpoint = endpoint;
}

const void *socket_packet_impl::data() const
{
    return m_buffer.data();
}

std::size_t socket_packet_impl::size() const
{
    return m_buffer.size();
}

message_category_t socket_packet_impl::category() const
{
    return message_category_t::packet;
}

message_subclass_t socket_packet_impl::subclass() const
{
    return message_net_class;
}

i_message::u_ptr_t socket_packet_impl::clone() const
{
    return create(m_buffer.fork()
                  , m_socket_endpoint);
}

const i_option *socket_packet_impl::options() const
{
    return &m_options;
}

transport_id_t socket_packet_impl::transport_id() const
{
    return m_socket_endpoint.transport_id;
}

bool socket_packet_impl::is_valid() const
{
    return !m_buffer.is_empty();
}

const socket_endpoint_t &socket_packet_impl::endpoint() const
{
    return m_socket_endpoint;
}

}
