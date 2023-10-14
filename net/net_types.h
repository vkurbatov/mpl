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
    dtls,
    ws,
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

}

#endif // MPL_NET_TYPES_H
