#ifndef MPL_NET_ICE_TRANSPORT_PARAMS_H
#define MPL_NET_ICE_TRANSPORT_PARAMS_H

#include "net/socket/socket_endpoint.h"
#include "ice_endpoint.h"

namespace mpl::net
{

struct ice_transport_params_t
{
    udp_endpoint_t::array_t     sockets;
    ice_component_id_t          component_id;
    ice_mode_t                  mode;
    ice_endpoint_t              local_endpoint;
    ice_endpoint_t              remote_endpoint;

    static std::string make_username(const std::string& lfrag
                                     , const std::string& rfrag);


    ice_transport_params_t(const udp_endpoint_t::array_t& sockets = {}
                           , ice_component_id_t component_id = 0
                           , ice_mode_t mode = ice_mode_t::undefined
                           , const ice_endpoint_t& local_endpoint = {}
                           , const ice_endpoint_t& remote_endpoint = {});

    bool is_full() const;

    std::string local_username() const;
    std::string remote_username() const;

    bool operator == (const ice_transport_params_t& other) const;
    bool operator != (const ice_transport_params_t& other) const;

    bool is_valid() const;
};

}

#endif // MPL_NET_ICE_TRANSPORT_PARAMS_H