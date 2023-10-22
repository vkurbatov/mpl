#ifndef SSL_CONNECTION_CONFIG_H
#define SSL_CONNECTION_CONFIG_H

#include "ssl_types.h"
#include <cstdint>

namespace pt::ssl
{

struct ssl_connection_config_t
{
    ssl_role_t              role;
    std::uint32_t           mtu;

    ssl_connection_config_t(ssl_role_t role = ssl_role_t::client
                            , std::uint32_t mtu = 0);

    bool can_handshake() const;

    bool is_valid() const;

};

}

#endif // SSL_CONNECTION_CONFIG_H
