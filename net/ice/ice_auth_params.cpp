#include "ice_auth_params.h"
#include "utils/common_utils.h"

namespace mpl::net
{


std::string ice_auth_params_t::generate_ufrag()
{
    return utils::random_string(4);
}

std::string ice_auth_params_t::generate_passwd()
{
    return utils::random_string(24);
}

ice_auth_params_t ice_auth_params_t::generate()
{
    return ice_auth_params_t(generate_ufrag()
                             , generate_passwd());
}


ice_auth_params_t::ice_auth_params_t(const std::string &ufrag
                                     , const std::string &password)
    : ufrag(ufrag)
    , password(password)
{

}

bool ice_auth_params_t::operator==(const ice_auth_params_t &other) const
{
    return ufrag == other.ufrag
            && password == other.password;
}

bool ice_auth_params_t::operator!=(const ice_auth_params_t &other) const
{
    return !operator==(other);
}


bool ice_auth_params_t::has_password() const
{
    return !password.empty();
}

bool ice_auth_params_t::is_defined() const
{
    return !ufrag.empty();
}

void ice_auth_params_t::tune(const ice_auth_params_t &other)
{
    if (ufrag.empty() != other.ufrag.empty())
    {
        if (other.ufrag.empty())
        {
            ufrag.clear();
        }
        else
        {
            ufrag = generate_ufrag();
        }
    }

    if (password.empty() != other.password.empty())
    {
        if (other.password.empty())
        {
            password.clear();
        }
        else
        {
            password = generate_passwd();
        }
    }

}

void ice_auth_params_t::clear()
{
    ufrag.clear();
    password.clear();
}

}
