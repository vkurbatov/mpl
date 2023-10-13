#ifndef MPL_NET_CONST_STUN_PACKET_H
#define MPL_NET_CONST_STUN_PACKET_H

#include "i_stun_packet.h"
#include "utils/smart_buffer_container.h"
#include "utils/option_container.h"

namespace mpl::net
{

struct stun_mapped_message_t;

class stun_packet_impl : public i_stun_packet
        , public utils::smart_buffer_container
        , public utils::option_container
{
    socket_address_t       m_address;

public:
    using u_ptr_t = std::unique_ptr<stun_packet_impl>;
    using s_ptr_t = std::shared_ptr<stun_packet_impl>;

    static u_ptr_t create(const smart_buffer& buffer
                          , const socket_address_t& address = {});
    static u_ptr_t create(smart_buffer&& buffer
                          , const socket_address_t& address = {});

    stun_packet_impl(const smart_buffer& buffer
                     , const socket_address_t& address = {});
    stun_packet_impl(smart_buffer&& buffer
                     , const socket_address_t& address = {});

    void set_address(const socket_address_t& address);

    // i_rtc_message interface
public:
    message_category_t category() const override;
    message_subclass_t subclass() const override;
    i_message::u_ptr_t clone() const override;

    // i_validable interface
public:
    bool is_valid() const override;

    // i_packet_data interface
public:
    const void *data() const override;
    std::size_t size() const override;

    // i_packet interface
public:
    transport_id_t transport_id() const override;
    const i_option* options() const override;
    const void *payload_data() const override;
    std::size_t payload_size() const override;

    // i_stun_packet interface
public:
    stun_message_class_t stun_class() const override;
    stun_method_t stun_method() const override;
    stun_transaction_id_t transaction_id() const override;
    stun_attribute_t::s_ptr_list_t attributes() const override;
    const socket_address_t& address() const override;

private:
    const stun_mapped_message_t& mapped_message() const;
    const std::uint8_t* map(std::int32_t index = 0) const;
};

}

#endif // MPL_NET_CONST_STUN_PACKET_H
