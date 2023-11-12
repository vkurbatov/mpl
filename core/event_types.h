#ifndef MPL_EVENT_TYPES_H
#define MPL_EVENT_TYPES_H

#include <cstdint>

namespace mpl
{

using event_id_t = std::uint32_t;

constexpr static event_id_t core_base_event_id = 0;
constexpr static event_id_t core_channel_state_event_id = core_base_event_id + 0;

}

#endif // MPL_EVENT_TYPES_H
