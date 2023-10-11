#ifndef MPL_NET_UDP_TRANSPORT_PARAMS_H
#define MPL_NET_UDP_TRANSPORT_PARAMS_H

#include "socket_endpoint.h"

namespace mpl::net
{

struct udp_transport_params_t
{
    socket_endpoint_t       local_endpoint;
    socket_endpoint_t       remote_endpoint;
    socket_options_t        options;

    udp_transport_params_t(const socket_endpoint_t& local_endpoint = {}
                           , const socket_endpoint_t& remote_endpoint = {}
                           , const socket_options_t& options = {});

    bool is_valid() const;
};

}

#endif // MPL_NET_UDP_TRANSPORT_PARAMS_H
