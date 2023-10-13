#include "ssl_connection_config.h"
#include "ssl_utils.h"

namespace ssl
{

ssl_connection_config_t::ssl_connection_config_t(ssl_role_t role
                                                 , std::uint32_t mtu)
    : role(role)
    , mtu(mtu)
{

}

bool ssl_connection_config_t::can_handshake() const
{
    return is_valid()
            && role != ssl_role_t::srvcli;
}

bool ssl_connection_config_t::is_valid() const
{
    return role != ssl_role_t::undefined
            && mtu > 0;
}


}
