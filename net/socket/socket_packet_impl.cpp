#include "socket_packet_impl.h"
#include "net/net_message_types.h"

namespace mpl::net
{

template class socket_packet_impl<transport_id_t::udp>;
template class socket_packet_impl<transport_id_t::tcp>;

template<transport_id_t Transport>
typename socket_packet_impl<Transport>::u_ptr_t socket_packet_impl<Transport>::create(const smart_buffer &packet_buffer
                                                                                      , const socket_address_t& address)
{
    return std::make_unique<socket_packet_impl>(packet_buffer
                                                , address);
}

template<transport_id_t Transport>
typename socket_packet_impl<Transport>::u_ptr_t socket_packet_impl<Transport>::create(smart_buffer &&packet_buffer
                                                                                      , const socket_address_t& address)
{
    return std::make_unique<socket_packet_impl>(std::move(packet_buffer)
                                                , address);
}

template<transport_id_t Transport>
socket_packet_impl<Transport>::socket_packet_impl(const smart_buffer &packet_buffer
                                                    , const socket_address_t& address)
    : smart_buffer_container(packet_buffer)
    , m_socket_address(address)
{

}

template<transport_id_t Transport>
socket_packet_impl<Transport>::socket_packet_impl(smart_buffer &&packet_buffer
                                                    , const socket_address_t& address)
    : smart_buffer_container(std::move(packet_buffer))
    , m_socket_address(address)
{

}

template<transport_id_t Transport>
void socket_packet_impl<Transport>::set_address(const socket_address_t& address)
{
    m_socket_address = address;
}

template<transport_id_t Transport>
const void *socket_packet_impl<Transport>::data() const
{
    return m_buffer.data();
}

template<transport_id_t Transport>
std::size_t socket_packet_impl<Transport>::size() const
{
    return m_buffer.size();
}

template<transport_id_t Transport>
message_category_t socket_packet_impl<Transport>::category() const
{
    return message_category_t::packet;
}

template<transport_id_t Transport>
message_subclass_t socket_packet_impl<Transport>::subclass() const
{
    return message_class_net;
}

template<transport_id_t Transport>
i_message::u_ptr_t socket_packet_impl<Transport>::clone() const
{
    return create(m_buffer.fork()
                  , m_socket_address);
}

template<transport_id_t Transport>
const i_option *socket_packet_impl<Transport>::options() const
{
    return &m_options;
}

template<transport_id_t Transport>
transport_id_t socket_packet_impl<Transport>::transport_id() const
{
    return Transport;
}

template<transport_id_t Transport>
bool socket_packet_impl<Transport>::is_valid() const
{
    return !m_buffer.is_empty();
}

template<transport_id_t Transport>
const socket_address_t &socket_packet_impl<Transport>::address() const
{
    return m_socket_address;
}

}
