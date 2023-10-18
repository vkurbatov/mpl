#include "utils/enum_converter_defs.h"
#include "net_types.h"
#include "socket/socket_types.h"
#include "ice/ice_types.h"

namespace mpl::utils
{

using namespace net;

__declare_enum_converter_begin(transport_id_t)
    __declare_enum_pair(transport_id_t, undefined),
    __declare_enum_pair_upper(transport_id_t, udp),
    __declare_enum_pair_upper(transport_id_t, tcp),
    __declare_enum_pair_upper(transport_id_t, ice),
    __declare_enum_pair_upper(transport_id_t, dtls),
    __declare_enum_pair_upper(transport_id_t, ws),
    __declare_enum_pair(transport_id_t, application)
__declare_enum_converter_end(transport_id_t)

__declare_enum_converter_begin(tcp_type_t)
    __declare_enum_pair(tcp_type_t, undefined),
    __declare_enum_pair(tcp_type_t, connection),
    __declare_enum_pair(tcp_type_t, listener)
__declare_enum_converter_end(tcp_type_t)

__declare_enum_converter_begin(role_t)
    __declare_enum_pair(role_t, undefined),
    __declare_enum_pair(role_t, passive),
    __declare_enum_pair(role_t, active),
    __declare_enum_pair(role_t, actpass),
    __declare_enum_pair(role_t, so),
__declare_enum_converter_end(role_t)

__declare_enum_converter_begin(ip_version_t)
    __declare_enum_pair(ip_version_t, undefined),
    __declare_enum_pair_upper(ip_version_t, ip4),
    __declare_enum_pair_upper(ip_version_t, ip6)
__declare_enum_converter_end(ip_version_t)

__declare_enum_converter_begin(ice_option_t)
    __declare_enum_pair(ice_option_t, trickle),
    __declare_enum_pair(ice_option_t, renomination)
__declare_enum_converter_end(ice_option_t)

__declare_enum_converter_begin(ice_candidate_type_t)
    __declare_enum_pair(ice_candidate_type_t, undefined),
    __declare_enum_pair(ice_candidate_type_t, host),
    __declare_enum_pair(ice_candidate_type_t, srflx),
    __declare_enum_pair(ice_candidate_type_t, prflx),
    __declare_enum_pair(ice_candidate_type_t, relay)
__declare_enum_converter_end(ice_candidate_type_t)

__declare_enum_converter_begin(ice_gathering_state_t)
    __declare_enum_pair(ice_gathering_state_t, undefined),
    __declare_enum_pair(ice_gathering_state_t, ready),
    __declare_enum_pair(ice_gathering_state_t, gathering),
    __declare_enum_pair(ice_gathering_state_t, completed),
    __declare_enum_pair(ice_gathering_state_t, closed),
    __declare_enum_pair(ice_gathering_state_t, failed)
__declare_enum_converter_end(ice_gathering_state_t)

__declare_enum_converter_begin(ice_mode_t)
    __declare_enum_pair(ice_mode_t, undefined),
    __declare_enum_pair(ice_mode_t, regular),
    __declare_enum_pair(ice_mode_t, aggressive),
    __declare_enum_pair(ice_mode_t, lite)
__declare_enum_converter_end(ice_mode_t)

}
