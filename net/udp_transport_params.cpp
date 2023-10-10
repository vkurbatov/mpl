#include "udp_transport_params.h"

namespace mpl::net
{

udp_transport_params_t::udp_transport_params_t(const socket_endpoint_t &local_endpoint
                                               , const socket_endpoint_t &remote_endpoint
                                               , bool reuse_address)
    : local_endpoint(local_endpoint)
    , remote_endpoint(remote_endpoint)
    , reuse_address(reuse_address)
{

}

bool udp_transport_params_t::is_valid() const
{
    return local_endpoint.is_valid()
            && local_endpoint.transport_id == transport_id_t::udp;
}


}
