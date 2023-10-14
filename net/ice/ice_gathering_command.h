#ifndef MPL_NET_ICE_GATHERING_COMMAND_H
#define MPL_NET_ICE_GATHERING_COMMAND_H

#include "core/command.h"
#include "net/net_command_types.h"
#include "ice_server_params.h"

namespace mpl::net
{

struct ice_gathering_command_t : public command_t
{
    constexpr static command_id_t       id = net_ice_gathering_command_id;
    constexpr static std::string_view   command_name = "ice_gathering";

    ice_server_params_t::array_t        ice_servers;

    ice_gathering_command_t(const ice_server_params_t::array_t& ice_servers = {});

};

}

#endif // MPL_NET_ICE_GATHERING_COMMAND_H
