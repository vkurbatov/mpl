#include "ssl_manager_config.h"
#include "ssl_utils.h"

namespace pt::ssl
{

srtp_profile_id_set_t ssl_manager_config_t::default_srtp_profiles =
{
    srtp_profile_id_t::aes_cm_128_hmac_sha1_80,
    srtp_profile_id_t::aes_cm_128_hmac_sha1_32,
    srtp_profile_id_t::aead_aes_128_gcm,
    srtp_profile_id_t::aead_aes_256_gcm
};

std::string ssl_manager_config_t::default_ciper_list = "DEFAULT:!NULL:!aNULL:!SHA256:!SHA384:!aECDH:!AESGCM+AES256:!aPSK";

ssl_manager_config_t::ssl_manager_config_t(const ssl_context_config_t &context_params
                                           , const std::string& certificate
                                           , const std::string& private_key
                                           , const std::string& subject
                                           , const std::string& ciper_list
                                           , const srtp_profile_id_set_t& srtp_profiles)
    : context_params(context_params)
    , certificate(certificate)
    , private_key(private_key)
    , subject(subject)
    , ciper_list(ciper_list)
    , srtp_profiles(srtp_profiles)
{

}

bool ssl_manager_config_t::has_srtp() const
{
    return !srtp_profiles.empty();
}

std::string ssl_manager_config_t::srtp_profile_list() const
{
    return to_string<srtp_profile_id_set_t>(srtp_profiles);
}

}
