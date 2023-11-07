#ifndef MPL_NET_SOCKET_PACKET_IMPL_H
#define MPL_NET_SOCKET_PACKET_IMPL_H

#include "i_socket_packet.h"
#include "utils/smart_buffer_container.h"
#include "utils/option_container.h"
#include "socket_endpoint.h"

namespace mpl::net
{

template<transport_id_t Transport>
class socket_packet_impl : public i_socket_packet
            , public utils::smart_buffer_container
            , public utils::option_container
{
    socket_address_t        m_socket_address;

public:

    using u_ptr_t = std::unique_ptr<socket_packet_impl>;
    using s_ptr_t = std::shared_ptr<socket_packet_impl>;

    static u_ptr_t create(const smart_buffer& packet_buffer
                          , const socket_address_t& address);

    static u_ptr_t create(smart_buffer&& packet_buffer = {}
                          , const socket_address_t& address = {});

    socket_packet_impl(const smart_buffer& packet_buffer
                       , const socket_address_t& address);

    socket_packet_impl(smart_buffer&& packet_buffer = {}
                       , const socket_address_t& address = {});

    void set_address(const socket_address_t& address);

    // i_data_object interface
public:
    const void *data() const override;
    std::size_t size() const override;

    // i_message interface
public:
    message_category_t category() const override;
    message_subclass_t subclass() const override;
    i_message::u_ptr_t clone() const override;

    // i_message_packet interface
public:
    const i_option *options() const override;

    // i_net_packet interface
public:
    transport_id_t transport_id() const override;
    bool is_valid() const override;

    // i_socket_packet interface
public:
    const socket_address_t &address() const override;
};

using udp_packet_impl = socket_packet_impl<transport_id_t::udp>;
using tcp_packet_impl = socket_packet_impl<transport_id_t::tcp>;

}

#endif // MPL_NET_SOCKET_PACKET_IMPL_H
