#ifndef MPL_NET_I_SOCKET_TRANSPORT_H
#define MPL_NET_I_SOCKET_TRANSPORT_H

#include "socket_endpoint.h"
#include "net/i_transport_channel.h"

namespace mpl::net
{


class i_socket_transport : public i_transport_channel
{
public:
    using u_ptr_t = std::unique_ptr<i_socket_transport>;
    using s_ptr_t = std::shared_ptr<i_socket_transport>;
    using w_ptr_t = std::weak_ptr<i_socket_transport>;
    using r_ptr_t = i_socket_transport*;

    virtual bool set_local_endpoint(const udp_endpoint_t& endpoint) = 0;
    virtual bool set_remote_endpoint(const udp_endpoint_t& endpoint) = 0;
    virtual udp_endpoint_t local_endpoint() const = 0;
    virtual udp_endpoint_t remote_endpoint() const = 0;
};

}

#endif // MPL_NET_I_SOCKET_TRANSPORT_H
