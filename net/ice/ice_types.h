#ifndef MPL_NET_ICE_TYPES_H
#define MPL_NET_ICE_TYPES_H

#include <cstdint>

namespace mpl::net
{

enum class ice_option_t
{
    trickle,
    renomination
};

enum class ice_candidate_type_t
{
    undefined = 0,
    host,
    srflx,
    prflx,
    relay
};


enum class ice_gathering_state_t
{
    undefined = 0,
    ready,
    gathering,
    completed,
    closed,
    failed
};

enum class ice_mode_t
{
    undefined = 0,
    regular,
    aggressive,
    lite
};

enum class ice_state_t
{
    frozen,
    waiting,
    in_progress,
    succeeded,
    failed,
    shutdown
};

using ice_component_id_t = std::uint8_t;

}

#endif // MPL_NET_ICE_TYPES_H
