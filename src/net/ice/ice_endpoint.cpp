#include "ice_endpoint.h"

namespace mpl::net
{

ice_endpoint_t::ice_endpoint_t(const ice_auth_params_t &auth
                               , const ice_candidate_t::array_t &candidates)
    : endpoint_t(transport_id_t::ice)
    , auth(auth)
    , candidates(candidates)
{

}

bool ice_endpoint_t::operator ==(const ice_endpoint_t &other) const
{
    return auth == other.auth
            && candidates == other.candidates;
}

bool ice_endpoint_t::operator ==(const endpoint_t &other) const
{
    return other.transport_id == transport_id_t::ice
            && *this == static_cast<const ice_endpoint_t&>(other);
}

bool ice_endpoint_t::is_valid() const
{
    return transport_id == transport_id_t::ice;
}

}
