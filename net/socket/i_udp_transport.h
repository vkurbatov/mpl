#ifndef MPL_NET_I_UDP_TRANSPORT_H
#define MPL_NET_I_UDP_TRANSPORT_H

#include "i_socket_transport.h"

namespace mpl::net
{

class i_udp_transport : public i_socket_transport
{
public:
    using u_ptr_t = std::unique_ptr<i_udp_transport>;
    using s_ptr_t = std::shared_ptr<i_udp_transport>;
    using w_ptr_t = std::weak_ptr<i_udp_transport>;
};

}

#endif // MPL_NET_I_UDP_TRANSPORT_H
