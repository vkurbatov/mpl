#include "udp_transport_params.h"

namespace mpl::net
{

udp_transport_params_t::udp_transport_params_t(const udp_endpoint_t &local_endpoint
                                               , const udp_endpoint_t &remote_endpoint
                                               , const socket_options_t& options)
    : local_endpoint(local_endpoint)
    , remote_endpoint(remote_endpoint)
    , options(options)
{

}

bool udp_transport_params_t::is_valid() const
{
    return local_endpoint.is_valid();
}


}
