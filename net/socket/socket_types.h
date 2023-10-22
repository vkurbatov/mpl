#ifndef MPL_NET_SOCKET_TYPES_H
#define MPL_NET_SOCKET_TYPES_H

#include "tools/io/net/ip_address.h"
#include "tools/io/net/ip_endpoint.h"
#include "tools/io/net/socket_options.h"

namespace mpl::net
{

enum class socket_type_t
{
    udp,
    tcp
};

using ip_version_t = pt::io::ip_version_t;
using ip_address_t = pt::io::ip_address_t;
using socket_address_t = pt::io::ip_endpoint_t;
using socket_options_t = pt::io::socket_options_t;

using socket_port_t = pt::io::port_t;
constexpr socket_port_t port_any = pt::io::port_any;
constexpr socket_port_t port_echo = pt::io::port_echo;
constexpr socket_port_t port_discard = pt::io::port_discard;
constexpr socket_port_t port_stun = 3478;

}

#endif // MPL_NET_SOCKET_TYPES_H
