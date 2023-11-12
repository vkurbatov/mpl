#ifndef SSL_SRTP_KEY_INFO_H
#define SSL_SRTP_KEY_INFO_H

#include "srtp_types.h"
#include <vector>
#include <string>
#include <cstdint>

namespace pt::ssl
{

using srtp_key_t = std::vector<std::uint8_t>;

struct srtp_key_info_t
{
    srtp_profile_id_t       profile_id;
    srtp_key_t              key;

    static std::size_t get_key_length(srtp_profile_id_t profile_id);
    static std::size_t get_salt_length(srtp_profile_id_t profile_id);
    static std::size_t get_master_length(srtp_profile_id_t profile_id);

    srtp_key_info_t(srtp_profile_id_t profile_id = srtp_profile_id_t::none
                    , const srtp_key_t& key = {});

    srtp_key_info_t(srtp_profile_id_t profile_id
                    , srtp_key_t&& key);

    bool operator == (const srtp_key_info_t& other) const;
    bool operator != (const srtp_key_info_t& other) const;

    bool is_valid() const;
    std::size_t key_length() const;
    std::size_t salt_length() const;
    std::size_t master_length() const;
};

}

#endif // SSL_SRTP_KEY_INFO_H
