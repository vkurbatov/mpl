#ifndef MPL_NET_UTILS_H
#define MPL_NET_UTILS_H

#include "core/channel_types.h"

namespace mpl::net
{

template<typename E>
channel_state_t get_channel_state(const E& other_state);

}

#endif // MPL_NET_UTILS_H
