#ifndef MPL_NET_I_ICE_TRANSPORT_H
#define MPL_NET_I_ICE_TRANSPORT_H

#include "net/i_transport_channel.h"
#include "net/socket/i_socket_transport.h"
#include "net/ice/ice_endpoint.h"
#include "ice_types.h"

namespace mpl::net
{

class i_ice_transport : public i_transport_channel
{
public:
    using u_ptr_t = std::unique_ptr<i_ice_transport>;
    using s_ptr_t = std::shared_ptr<i_ice_transport>;

    virtual ice_component_id_t component_id() const = 0;
    virtual ice_mode_t mode() const = 0;
    virtual ice_gathering_state_t gathering_state() const = 0;
    virtual ice_endpoint_t local_endpoint() const = 0;
    virtual ice_endpoint_t remote_endpoint() const = 0;
    virtual bool add_remote_candidate(const ice_candidate_t& candidate) = 0;
};

}

#endif // MPL_NET_I_ICE_TRANSPORT_H
