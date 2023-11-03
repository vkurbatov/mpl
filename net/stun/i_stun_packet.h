#ifndef MPL_NET_I_STUN_PACKET_H
#define MPL_NET_I_STUN_PACKET_H

#include "net/socket/i_socket_packet.h"
#include "stun_types.h"
#include "stun_attributes.h"
#include "net/socket/socket_endpoint.h"

namespace mpl::net
{

class i_stun_packet : public i_socket_packet
{
public:
    using s_ptr_t = std::shared_ptr<i_stun_packet>;
    virtual stun_message_class_t stun_class() const = 0;
    virtual stun_method_t stun_method() const = 0;
    virtual stun_transaction_id_t transaction_id() const = 0;
    virtual stun_attribute_t::s_ptr_list_t attributes() const = 0;
    virtual const void* payload_data() const = 0;
    virtual std::size_t payload_size() const = 0;

};

}

#endif // MPL_NET_I_STUN_PACKET_H
