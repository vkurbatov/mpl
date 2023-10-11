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

using ip_version_t = io::ip_version_t;
using ip_address_t = io::ip_address_t;
using ip_endpoint_t = io::ip_endpoint_t;
using socket_options_t = io::socket_options_t;

using socket_port_t = io::port_t;
constexpr socket_port_t port_any = io::port_any;
constexpr socket_port_t port_echo = io::port_echo;
constexpr socket_port_t port_discard = io::port_discard;

}

#endif // MPL_NET_SOCKET_TYPES_H
