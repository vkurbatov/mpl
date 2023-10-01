#include "enum_converter_defs.h"

#include "core/channel_types.h"

namespace mpl::utils
{

declare_enum_converter_begin(channel_state_t)
    declare_pair(channel_state_t, undefined),
    declare_pair(channel_state_t, ready),
    declare_pair(channel_state_t, create),
    declare_pair(channel_state_t, opening),
    declare_pair(channel_state_t, open),
    declare_pair(channel_state_t, starting),
    declare_pair(channel_state_t, started),
    declare_pair(channel_state_t, connecting),
    declare_pair(channel_state_t, connected),
    declare_pair(channel_state_t, stopping),
    declare_pair(channel_state_t, stopped),
    declare_pair(channel_state_t, disconnecting),
    declare_pair(channel_state_t, disconnected),
    declare_pair(channel_state_t, closing),
    declare_pair(channel_state_t, closed),
    declare_pair(channel_state_t, failed),
    declare_pair(channel_state_t, remove)
declare_enum_converter_end(channel_state_t)

}
