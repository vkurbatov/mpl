#include "ice_transport_params.h"

namespace mpl::net
{

std::string ice_transport_params_t::make_username(const std::string &lfrag
                                                  , const std::string &rfrag)
{
    if (!lfrag.empty()
            && !rfrag.empty())
    {
        return std::string(lfrag)
                .append(":")
                .append(rfrag);
    }

    return rfrag;
}


ice_transport_params_t::ice_transport_params_t(const udp_endpoint_t::array_t &sockets
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

bool ice_transport_params_t::is_full() const
{
    switch(mode)
    {
        case ice_mode_t::aggressive:
        case ice_mode_t::regular:
            return true;
        break;
        default:;
    }

    return false;
}

std::string ice_transport_params_t::local_username() const
{
    return make_username(local_endpoint.auth.ufrag
                         , remote_endpoint.auth.ufrag);
}

std::string ice_transport_params_t::remote_username() const
{
    return make_username(remote_endpoint.auth.ufrag
                         , local_endpoint.auth.ufrag);
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
