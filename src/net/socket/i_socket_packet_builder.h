#ifndef MPL_NET_I_SOCKET_PACKET_BUILDER_H
#define MPL_NET_I_SOCKET_PACKET_BUILDER_H

#include "socket_types.h"
#include "net/i_net_packet_builder.h"

namespace mpl::net
{

class i_socket_packet_builder : public i_net_packet_builder
{
public:
    using u_ptr_t = std::unique_ptr<i_socket_packet_builder>;
    using s_ptr_t = std::shared_ptr<i_socket_packet_builder>;

    virtual void set_address(const socket_address_t& address) = 0;
    virtual const socket_address_t& address() const = 0;
};

}

#endif // MPL_NET_I_SOCKET_PACKET_BUILDER_H
