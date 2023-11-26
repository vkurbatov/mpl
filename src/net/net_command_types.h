#ifndef MPL_NET_COMMAND_TYPES_H
#define MPL_NET_COMMAND_TYPES_H

#include "core/command_types.h"

namespace mpl::net
{

constexpr command_id_t net_base_command_id = core_base_command_id + 2000;
constexpr command_id_t net_ice_gathering_command_id = net_base_command_id + 0;

}

#endif // MPL_NET_COMMAND_TYPES_H
