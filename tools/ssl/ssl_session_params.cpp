#include "ssl_session_params.h"

namespace ssl
{

ssl_session_params_t::ssl_session_params_t(ssl_role_t role
                                           , uint32_t mtu)
    : role(role)
    , mtu(mtu)
{

}

bool ssl_session_params_t::operator ==(const ssl_session_params_t &other) const
{
    return role == other.role
            && mtu == other.mtu;
}

bool ssl_session_params_t::operator !=(const ssl_session_params_t &other) const
{
    return ! operator == (other);
}

bool ssl_session_params_t::is_valid() const
{
    return role != ssl_role_t::undefined
            && mtu > 0;
}

}
