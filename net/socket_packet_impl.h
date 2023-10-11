#ifndef MPL_NET_SOCKET_PACKET_IMPL_H
#define MPL_NET_SOCKET_PACKET_IMPL_H

#include "i_socket_packet.h"
#include "utils/smart_buffer.h"
#include "utils/option_impl.h"
#include "socket_endpoint.h"

namespace mpl::net
{

class socket_packet_impl : public i_socket_packet
{
    smart_buffer            m_packet_buffer;
    socket_endpoint_t       m_socket_endpoint;

public:

    using u_ptr_t = std::unique_ptr<socket_packet_impl>;
    using s_ptr_t = std::shared_ptr<socket_packet_impl>;

    static u_ptr_t create(const smart_buffer& packet_buffer
                          , const socket_endpoint_t& endpoint);

    static u_ptr_t create(smart_buffer&& packet_buffer
                          , const socket_endpoint_t& endpoint);

    socket_packet_impl(const smart_buffer& packet_buffer
                       , const socket_endpoint_t& endpoint);

    socket_packet_impl(smart_buffer&& packet_buffer = {}
                       , const socket_endpoint_t& endpoint = {});

    smart_buffer& buffer();
    const smart_buffer& buffer() const;

    void set_endpoint(const socket_endpoint_t& endpoint);

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
    const socket_endpoint_t &endpoint() const override;
};

}

#endif // MPL_NET_SOCKET_PACKET_IMPL_H
