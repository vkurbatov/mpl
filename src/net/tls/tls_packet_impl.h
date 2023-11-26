#ifndef MPL_NET_TLS_PACKET_IMPL_H
#define MPL_NET_TLS_PACKET_IMPL_H

#include "i_tls_packet.h"
#include "utils/smart_buffer_container.h"
#include "utils/option_container.h"
#include "net/socket/socket_types.h"

namespace mpl::net
{

class tls_packet_impl : public i_tls_packet
        , public utils::smart_buffer_container
        , public utils::option_container
{
    socket_address_t    m_socket_address;
public:
    using u_ptr_t = std::unique_ptr<tls_packet_impl>;
    using s_ptr_t = std::shared_ptr<tls_packet_impl>;

    static u_ptr_t create(const smart_buffer& buffer
                          , const socket_address_t& socket_address = {});
    static u_ptr_t create(smart_buffer&& buffer
                          , const socket_address_t& socket_address = {});

    tls_packet_impl(const smart_buffer& buffer
                    , const socket_address_t& socket_address = {});
    tls_packet_impl(smart_buffer&& buffer
                    , const socket_address_t& socket_address = {});

    // i_data_object interface
public:
    const void *data() const override;
    std::size_t size() const override;

    // i_message interface
public:
    message_category_t category() const override;
    module_id_t module_id() const override;
    i_message::u_ptr_t clone() const override;

    // i_message_packet interface
public:
    const i_option *options() const override;

    // i_net_packet interface
public:
    transport_id_t transport_id() const override;
    bool is_valid() const override;

    // i_tls_packet interface
public:
    uint64_t sequension_number() const override;
    content_type_t content_type() const override;
    const socket_address_t &address() const override;
};

}

#endif // MPL_NET_TLS_PACKET_IMPL_H
