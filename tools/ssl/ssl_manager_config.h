#ifndef SSL_MANAGER_CONFIG_H
#define SSL_MANAGER_CONFIG_H

#include "ssl_context_config.h"
#include "srtp_types.h"
#include <string>

namespace ssl
{


struct ssl_manager_config_t
{
    static srtp_profile_id_set_t default_srtp_profiles;
    static std::string default_ciper_list;

    ssl_context_config_t    context_params;
    std::string             certificate;
    std::string             private_key;
    std::string             subject; // for generate cert and private key
    std::string             ciper_list;
    srtp_profile_id_set_t   srtp_profiles;

    ssl_manager_config_t(const ssl_context_config_t& context_params = {}
                         , const std::string& certificate = {}
                         , const std::string& private_key = {}
                         , const std::string& subject = {}
                         , const std::string& ciper_list = {}
                         , const srtp_profile_id_set_t& srtp_profiles = {});

    bool has_srtp() const;
    std::string srtp_profile_list() const;
};

}

#endif // SSL_MANAGER_CONFIG_H
