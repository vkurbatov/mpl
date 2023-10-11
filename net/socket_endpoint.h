#ifndef MPL_NET_SOCKET_ENDPOINT_H
#define MPL_NET_SOCKET_ENDPOINT_H

#include "socket_types.h"
#include "endpoint.h"

namespace mpl::net
{

struct socket_endpoint_t : public endpoint_t
{
    ip_endpoint_t       ip_endpoint;

    socket_endpoint_t(socket_type_t socket_type = socket_type_t::udp
                      , const ip_endpoint_t& ip_endpoint = {});

    // endpoint_t interface
public:
    bool operator ==(const endpoint_t &other) const override;
    bool is_valid() const override;
};

}

#endif // MPL_NET_SOCKET_ENDPOINT_H
