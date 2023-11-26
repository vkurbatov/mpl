#ifndef MPL_NET_UTILS_H
#define MPL_NET_UTILS_H

#include "core/channel_types.h"
#include "net_types.h"
#include <cstdint>

namespace mpl::utils
{

template<typename E>
channel_state_t get_channel_state(const E& other_state);

net::protocol_type_t parse_protocol(const void *data, std::size_t size);

}

#endif // MPL_NET_UTILS_H
