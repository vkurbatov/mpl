#ifndef IO_NET_TYPES_H
#define IO_NET_TYPES_H

#include <cstdint>

namespace pt::io
{


enum class ip_version_t
{
    undefined,
    ip4,
    ip6
};

enum class role_t
{
    undefined = 0,
    server,
    client,
    srvcli
};

using port_t = std::uint16_t;
constexpr port_t port_any = 0;
constexpr port_t port_echo = 5;
constexpr port_t port_discard = 9;

}

#endif // IO_NET_TYPES_H
