#ifndef MPL_NET_ICE_AUTH_PARAMS_H
#define MPL_NET_ICE_AUTH_PARAMS_H

#include <string>

namespace mpl::net
{

struct ice_auth_params_t
{
    std::string     ufrag;
    std::string     password;

    static std::string generate_ufrag();
    static std::string generate_passwd();
    static ice_auth_params_t generate();

    ice_auth_params_t(const std::string& ufrag = {}
                      , const std::string& password = {});

    bool operator==(const ice_auth_params_t& other) const;
    bool operator!=(const ice_auth_params_t& other) const;

    bool has_password() const;
    bool is_defined() const;

    void tune(const ice_auth_params_t& other);
    void clear();
};

}

#endif // MPL_NET_ICE_AUTH_PARAMS_H
