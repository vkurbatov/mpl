#ifndef MPL_NET_TYPES_H
#define MPL_NET_TYPES_H

#include "core/message_types.h"

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
    active,
    passive,
    actpass
};

constexpr message_subclass_t message_net_class = message_core_class + 2;

}

#endif // MPL_NET_TYPES_H
