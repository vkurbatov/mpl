#include "utils/enum_serialize_defs.h"

#include "net_types.h"
#include "socket/socket_types.h"
#include "ice/ice_types.h"

namespace mpl
{

using namespace net;

__declare_enum_serializer(transport_id_t)
__declare_enum_serializer(tcp_type_t)
__declare_enum_serializer(role_t)
__declare_enum_serializer(ip_version_t)

__declare_enum_serializer(ice_option_t)
__declare_enum_serializer(ice_candidate_type_t)
__declare_enum_serializer(ice_gathering_state_t)
__declare_enum_serializer(ice_mode_t)

}
