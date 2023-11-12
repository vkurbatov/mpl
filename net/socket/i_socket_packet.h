#ifndef MPL_NET_I_SOCKET_PACKET_H
#define MPL_NET_I_SOCKET_PACKET_H

#include "net/i_net_packet.h"
#include "socket_endpoint.h"

namespace mpl::net
{

class i_socket_packet : public i_net_packet
{
public:
    using u_ptr_t = std::unique_ptr<i_socket_packet>;
    using s_ptr_t = std::shared_ptr<i_socket_packet>;

    virtual const socket_address_t& address() const = 0;
};


}

#endif // MPL_NET_I_SOCKET_PACKET_H
