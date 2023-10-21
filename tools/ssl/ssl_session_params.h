#ifndef SSL_SESSION_PARAMS_H
#define SSL_SESSION_PARAMS_H

#include "ssl_types.h"
#include <cstdint>

namespace ssl
{

struct ssl_session_params_t
{
    constexpr static std::uint32_t default_mtu_size = 1500;

    ssl_role_t              role;
    std::uint32_t           mtu;

    ssl_session_params_t(ssl_role_t role = ssl_role_t::srvcli
                         , std::uint32_t mtu = default_mtu_size);

    bool operator == (const ssl_session_params_t& other) const;
    bool operator != (const ssl_session_params_t& other) const;

    bool is_valid() const;
};

}

#endif // SSL_SESSION_PARAMS_H
