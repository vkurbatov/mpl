#ifndef MPL_NET_EVENT_TYPES_H
#define MPL_NET_EVENT_TYPES_H

#include "core/event_types.h"

namespace mpl::net
{

constexpr static event_id_t net_base_event_id = core_base_event_id + 2000;
constexpr static event_id_t net_ice_gathering_state_event_id = net_base_event_id + 0;

}

#endif // MPL_NET_EVENT_TYPES_H
