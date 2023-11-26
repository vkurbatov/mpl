#include "tls_transport_params.h"

namespace mpl::net
{

tls_transport_params_t::tls_transport_params_t(role_t role
                                               , const tls_endpoint_t &local_endpoint
                                               , const tls_endpoint_t &remote_endpoint)
    : role(role)
    , local_endpoint(local_endpoint)
    , remote_endpoint(remote_endpoint)
{

}

bool tls_transport_params_t::operator ==(const tls_transport_params_t &other) const
{
    return role == other.role
            && local_endpoint == other.local_endpoint
            && remote_endpoint == other.remote_endpoint;
}

bool tls_transport_params_t::operator !=(const tls_transport_params_t &other) const
{
    return ! operator == (other);
}

bool tls_transport_params_t::is_valid() const
{
    return role != role_t::undefined
            && local_endpoint.is_valid();
}

}
