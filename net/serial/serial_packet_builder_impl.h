#ifndef MPL_NET_SERIAL_PACKET_BUILDER_IMPL_H
#define MPL_NET_SERIAL_PACKET_BUILDER_IMPL_H

#include "i_serial_packet_builder.h"
#include "utils/smart_buffer.h"

namespace mpl::net
{

class serial_packet_builder_impl : public i_serial_packet_builder
{
    std::string         m_port_name;
    smart_buffer        m_packet_buffer;

public:

    using u_ptr_t = std::unique_ptr<serial_packet_builder_impl>;
    using s_ptr_t = std::shared_ptr<serial_packet_builder_impl>;

    static u_ptr_t create(const std::string_view& port_name = {}
                          , const void* packet_data = nullptr
                          , std::size_t packet_size = 0);

    serial_packet_builder_impl(const std::string_view& port_name = {}
                              , const void* packet_data = nullptr
                              , std::size_t packet_size = 0);

    // i_net_packet_builder interface
public:
    transport_id_t transport_id() const override;
    void set_packet_data(const void *packet_data, std::size_t packet_size) override;
    i_net_packet::u_ptr_t build_packet() override;

    // i_serial_packet_builder interface
public:
    void set_port_name(const std::string_view &port_name) override;
    std::string port_name() const override;
};

}

#endif // MPL_NET_SERIAL_PACKET_BUILDER_IMPL_H
