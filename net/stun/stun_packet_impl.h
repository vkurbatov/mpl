#ifndef MPL_NET_CONST_STUN_PACKET_H
#define MPL_NET_CONST_STUN_PACKET_H

#include "i_stun_packet.h"
#include "rtc/rtc_buffer_container.h"
#include "rtc/rtc_option_container.h"

namespace mpl::net
{

struct stun_mapped_message_t;

class stun_packet_impl : public i_stun_packet
        , public rtc_buffer_container
        , public rtc_option_container
{
public:
    using u_ptr_t = std::unique_ptr<stun_packet_impl>;
    using s_ptr_t = std::shared_ptr<stun_packet_impl>;

    static u_ptr_t create(const smart_buffer& buffer);
    static u_ptr_t create(smart_buffer&& buffer);

    stun_packet_impl(const smart_buffer& buffer);
    stun_packet_impl(smart_buffer&& buffer);

    // i_rtc_message interface
public:
    rtc_message_category_t category() const override;
    i_rtc_message::u_ptr_t clone() const override;

    // i_validable interface
public:
    bool is_valid() const override;

    // i_packet_data interface
public:
    const void *data() const override;
    std::size_t size() const override;

    // i_packet interface
public:
    rtc_packet_type_t packet_type() const override;
    const i_option& options() const override;
    const void *payload_data() const override;
    std::size_t payload_size() const override;

    // i_stun_packet interface
public:
    stun_message_class_t stun_class() const override;
    stun_method_t stun_method() const override;
    stun_transaction_id_t transaction_id() const override;
    stun_attribute_t::w_ptr_list_t attributes() const override;
    stun_authentification_result_t check_authentification(const std::string &password) const override;

private:
    const stun_mapped_message_t& mapped_message() const;
    const std::uint8_t* map(std::int32_t index = 0) const;
};

}

#endif // MPL_NET_CONST_STUN_PACKET_H
