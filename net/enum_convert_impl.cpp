#include "utils/enum_converter_defs.h"
#include "net_types.h"
#include "socket/socket_types.h"

namespace mpl::utils
{

using namespace net;

__declare_enum_converter_begin(transport_id_t)
    __declare_enum_pair(transport_id_t, undefined),
    __declare_enum_pair_name(transport_id_t, udp, "UDP"),
    __declare_enum_pair_name(transport_id_t, tcp, "TCP"),
    __declare_enum_pair_name(transport_id_t, ice, "ICE"),
    __declare_enum_pair_name(transport_id_t, dtls, "DTLS"),
    __declare_enum_pair_name(transport_id_t, ws, "WS"),
    __declare_enum_pair(transport_id_t, application)
__declare_enum_converter_end(transport_id_t)

__declare_enum_converter_begin(tcp_type_t)
    __declare_enum_pair(tcp_type_t, undefined),
    __declare_enum_pair(tcp_type_t, connection),
    __declare_enum_pair(tcp_type_t, listener)
__declare_enum_converter_end(tcp_type_t)

__declare_enum_converter_begin(role_t)
    __declare_enum_pair(role_t, undefined),
    __declare_enum_pair(role_t, active),
    __declare_enum_pair(role_t, passive),
    __declare_enum_pair(role_t, actpass)
__declare_enum_converter_end(role_t)

__declare_enum_converter_begin(ip_version_t)
    __declare_enum_pair(ip_version_t, undefined),
    __declare_enum_pair_name(ip_version_t, ip4, "IP4"),
    __declare_enum_pair_name(ip_version_t, ip6, "IP6")
__declare_enum_converter_end(ip_version_t)

}
