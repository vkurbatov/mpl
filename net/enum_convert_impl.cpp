#include "utils/enum_converter_defs.h"
#include "net_types.h"
#include "socket/socket_types.h"
#include "ice/ice_types.h"
#include "tls/tls_types.h"
#include "serial/serial_types.h"

namespace mpl::utils
{

using namespace net;

__declare_enum_converter_begin(transport_id_t)
    __declare_enum_pair(transport_id_t, undefined),
    __declare_enum_pair(transport_id_t, udp),
    __declare_enum_pair(transport_id_t, tcp),
    __declare_enum_pair(transport_id_t, ice),
    __declare_enum_pair(transport_id_t, tls),
    __declare_enum_pair(transport_id_t, ws),
    __declare_enum_pair(transport_id_t, serial),
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
    __declare_enum_pair(ip_version_t, ip4),
    __declare_enum_pair(ip_version_t, ip6)
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

__declare_enum_converter_begin(tls_hash_method_t)
    __declare_enum_pair(tls_hash_method_t, undefined),
    __declare_enum_pair(tls_hash_method_t, md4),
    __declare_enum_pair(tls_hash_method_t, md5),
    __declare_enum_pair(tls_hash_method_t, sha_1),
    __declare_enum_pair(tls_hash_method_t, sha_224),
    __declare_enum_pair(tls_hash_method_t, sha_256),
    __declare_enum_pair(tls_hash_method_t, sha_384),
    __declare_enum_pair(tls_hash_method_t, sha_512),
    __declare_enum_pair(tls_hash_method_t, md5_sha1)
__declare_enum_converter_end(tls_hash_method_t)

__declare_enum_converter_begin(tls_method_t)
    __declare_enum_pair(tls_method_t, dtls),
    __declare_enum_pair(tls_method_t, tls)
__declare_enum_converter_end(tls_method_t)

__declare_enum_converter_begin(serial_parity_t)
    __declare_enum_pair(serial_parity_t, none),
    __declare_enum_pair(serial_parity_t, odd),
    __declare_enum_pair(serial_parity_t, even)
__declare_enum_converter_end(serial_parity_t)

__declare_enum_converter_begin(serial_stop_bits_t)
    __declare_enum_pair(serial_stop_bits_t, one),
    __declare_enum_pair(serial_stop_bits_t, onepointfive),
    __declare_enum_pair(serial_stop_bits_t, two)
__declare_enum_converter_end(serial_stop_bits_t)

__declare_enum_converter_begin(serial_flow_control_t)
    __declare_enum_pair(serial_flow_control_t, none),
    __declare_enum_pair(serial_flow_control_t, software),
    __declare_enum_pair(serial_flow_control_t, hardware)
__declare_enum_converter_end(serial_flow_control_t)

}
