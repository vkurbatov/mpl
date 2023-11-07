#ifndef MPL_NET_SOCKET_PACKET_BUILDER_IMPL_H
#define MPL_NET_SOCKET_PACKET_BUILDER_IMPL_H

#include "i_socket_packet_builder.h"
#include "utils/smart_buffer.h"

namespace mpl::net
{

class socket_packet_builder_impl : public i_socket_packet_builder
{
    transport_id_t      m_transport_id;
    socket_address_t    m_socket_address;
    smart_buffer        m_packet_buffer;

public:
    using u_ptr_t = std::unique_ptr<socket_packet_builder_impl>;
    using s_ptr_t = std::shared_ptr<socket_packet_builder_impl>;

    static u_ptr_t create(transport_id_t transport_id
                          , const socket_address_t& socket_address = {}
                          , const void* packet_data = nullptr
                          , std::size_t packet_size = 0);

    socket_packet_builder_impl(transport_id_t transport_id
                            , const socket_address_t& socket_address = {}
                            , const void* packet_data = nullptr
                            , std::size_t packet_size = 0);

    // i_net_packet_builder interface
public:
    transport_id_t transport_id() const override;
    void set_packet_data(const void* packet_data
                         , std::size_t packet_size) override;
    const i_data_object &packet_data() const override;

    i_net_packet::u_ptr_t build_packet() override;

    // i_socket_packet_builder interface
public:
    void set_address(const socket_address_t &address) override;
    const socket_address_t &address() const override;

};

}

#endif // MPL_NET_SOCKET_PACKET_BUILDER_IMPL_H
