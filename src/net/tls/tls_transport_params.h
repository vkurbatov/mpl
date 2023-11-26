#ifndef MPL_NET_TLS_TRANSPORT_PARAMS_H
#define MPL_NET_TLS_TRANSPORT_PARAMS_H

#include "net/net_types.h"
#include "tls_endpoint.h"

namespace mpl::net
{

struct tls_transport_params_t
{
    role_t              role;
    tls_endpoint_t      local_endpoint;
    tls_endpoint_t      remote_endpoint;

    tls_transport_params_t(role_t role = role_t::undefined
                            , const tls_endpoint_t& local_endpoint = {}
                            , const tls_endpoint_t& remote_endpoint = {});

    bool operator == (const tls_transport_params_t& other) const;
    bool operator != (const tls_transport_params_t& other) const;

    bool is_valid() const;


};

}

#endif // MPL_NET_TLS_TRANSPORT_PARAMS_H
