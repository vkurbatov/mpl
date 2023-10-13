#include "ice_transport_params.h"

namespace mpl::net
{

ice_transport_params_t::ice_transport_params_t(const socket_endpoint_t::array_t &sockets
                                               , ice_component_id_t component_id
                                               , ice_mode_t mode
                                               , const ice_endpoint_t &local_endpoint
                                               , const ice_endpoint_t &remote_endpoint)
    : sockets(sockets)
    , component_id(component_id)
    , mode(mode)
    , local_endpoint(local_endpoint)
    , remote_endpoint(remote_endpoint)
{

}

bool ice_transport_params_t::operator ==(const ice_transport_params_t &other) const
{
    return sockets == other.sockets
            && component_id == other.component_id
            && mode == other.mode
            && local_endpoint == other.local_endpoint
            && remote_endpoint == other.remote_endpoint;
}

bool ice_transport_params_t::operator !=(const ice_transport_params_t &other) const
{
    return ! operator == (other);
}

bool ice_transport_params_t::is_valid() const
{
    return true;
}



}
