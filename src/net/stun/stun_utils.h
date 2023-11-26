#ifndef MPL_NET_STUN_UTILS_H
#define MPL_NET_STUN_UTILS_H

#include "core/i_data_object.h"
#include "stun_types.h"
#include <string_view>

namespace mpl::utils
{

net::stun_authentification_result_t stun_verify_auth(const std::string_view& password
                                                     , const i_data_object& packet_data);

}

#endif // MPL_NET_STUN_UTILS_H
