#include "serial_packet_impl.h"
#include "net/net_message_types.h"

namespace mpl::net
{

serial_packet_impl::u_ptr_t serial_packet_impl::create(const smart_buffer &packet_buffer
                                                       , const std::string_view &port_name)
{
    return std::make_unique<serial_packet_impl>(packet_buffer
                                                , port_name);
}

serial_packet_impl::u_ptr_t serial_packet_impl::create(smart_buffer &&packet_buffer
                                                       , const std::string_view &port_name)
{
    return std::make_unique<serial_packet_impl>(std::move(packet_buffer)
                                                , port_name);
}

serial_packet_impl::serial_packet_impl(const smart_buffer &packet_buffer
                                       , const std::string_view &port_name)
    : smart_buffer_container(packet_buffer)
    , m_port_name(port_name)
{

}

serial_packet_impl::serial_packet_impl(smart_buffer &&packet_buffer
                                       , const std::string_view &port_name)
    : smart_buffer_container(std::move(packet_buffer))
    , m_port_name(port_name)
{

}

const void *serial_packet_impl::data() const
{
    return m_buffer.data();
}

std::size_t serial_packet_impl::size() const
{
    return m_buffer.size();
}

message_category_t serial_packet_impl::category() const
{
    return message_category_t::packet;
}

message_subclass_t serial_packet_impl::subclass() const
{
    return message_class_net;
}

i_message::u_ptr_t serial_packet_impl::clone() const
{
    return create(m_buffer.fork()
                  , m_port_name);
}

const i_option *serial_packet_impl::options() const
{
    return &m_options;
}

transport_id_t serial_packet_impl::transport_id() const
{
    return transport_id_t::serial;
}

bool serial_packet_impl::is_valid() const
{
    return !m_buffer.is_empty();
}

std::string serial_packet_impl::port_name() const
{
    return m_port_name;
}


}
