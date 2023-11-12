#include "enum_converter_defs.h"

#include "core/channel_types.h"

namespace mpl::utils
{

__declare_enum_converter_begin(channel_state_t)
    __declare_enum_pair(channel_state_t, undefined),
    __declare_enum_pair(channel_state_t, ready),
    __declare_enum_pair(channel_state_t, create),
    __declare_enum_pair(channel_state_t, opening),
    __declare_enum_pair(channel_state_t, open),
    __declare_enum_pair(channel_state_t, starting),
    __declare_enum_pair(channel_state_t, started),
    __declare_enum_pair(channel_state_t, connecting),
    __declare_enum_pair(channel_state_t, connected),
    __declare_enum_pair(channel_state_t, stopping),
    __declare_enum_pair(channel_state_t, stopped),
    __declare_enum_pair(channel_state_t, disconnecting),
    __declare_enum_pair(channel_state_t, disconnected),
    __declare_enum_pair(channel_state_t, closing),
    __declare_enum_pair(channel_state_t, closed),
    __declare_enum_pair(channel_state_t, failed),
    __declare_enum_pair(channel_state_t, remove)
__declare_enum_converter_end(channel_state_t)

}
