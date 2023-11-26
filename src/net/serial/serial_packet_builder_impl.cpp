#include "serial_packet_builder_impl.h"
#include "serial_packet_impl.h"

namespace mpl::net
{

serial_packet_builder_impl::u_ptr_t serial_packet_builder_impl::create(const std::string_view &port_name
                                                                       , const void *packet_data
                                                                       , std::size_t packet_size)
{
    return std::make_unique<serial_packet_builder_impl>(port_name
                                                        , packet_data
                                                        , packet_size);
}

serial_packet_builder_impl::serial_packet_builder_impl(const std::string_view &port_name
                                                       , const void *packet_data
                                                       , std::size_t packet_size)
    : m_port_name(port_name)
    , m_packet_buffer(packet_data
                      , packet_size
                      , true)
{

}

transport_id_t serial_packet_builder_impl::transport_id() const
{
    return transport_id_t::serial;
}

void serial_packet_builder_impl::set_packet_data(const void *packet_data, std::size_t packet_size)
{
    m_packet_buffer.assign(packet_data
                           , packet_size
                           , true);
}

const i_data_object &serial_packet_builder_impl::packet_data() const
{
    return m_packet_buffer;
}

i_net_packet::u_ptr_t serial_packet_builder_impl::build_packet()
{
    return serial_packet_impl::create(m_packet_buffer.fork()
                                      , m_port_name);
}

void serial_packet_builder_impl::set_port_name(const std::string_view &port_name)
{
    m_port_name = port_name;
}

std::string serial_packet_builder_impl::port_name() const
{
    return m_port_name;
}


}
