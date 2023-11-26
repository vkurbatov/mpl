#ifndef MPL_NET_ICE_ENDPOINT_H
#define MPL_NET_ICE_ENDPOINT_H

#include "net/endpoint.h"
#include "ice_auth_params.h"
#include "ice_candidate.h"

namespace mpl::net
{

struct ice_endpoint_t : public endpoint_t
{
    ice_auth_params_t           auth;
    ice_candidate_t::array_t    candidates;

    ice_endpoint_t(const ice_auth_params_t& auth = {}
                   , const ice_candidate_t::array_t& candidates = {});

    bool operator == (const ice_endpoint_t& other) const;

    // endpoint_t interface
public:
    bool operator ==(const endpoint_t &other) const override;
    bool is_valid() const override;
};

}

#endif // MPL_NET_ICE_ENDPOINT_H
