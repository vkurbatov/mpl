#ifndef MPL_NET_TYPES_H
#define MPL_NET_TYPES_H

namespace mpl::net
{

enum class transport_id_t
{
    undefined = 0,
    udp,
    tcp,
    ice,
    tls,
    ws,
    serial,
    application
};

enum class tcp_type_t
{
    undefined = 0,
    connection,
    listener
};

enum class role_t
{
    undefined = 0,
    passive,
    active,
    actpass,
    so
};

enum class protocol_type_t
{
    undefined,
    stun,
    tls,
    rtp,
    rtcp
};

}

#endif // MPL_NET_TYPES_H