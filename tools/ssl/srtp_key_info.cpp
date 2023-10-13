#include "srtp_key_info.h"
#include <srtp2/srtp.h>

namespace ssl
{

std::size_t srtp_key_info_t::get_key_length(srtp_profile_id_t profile_id)
{
    return srtp_profile_get_master_key_length(static_cast<srtp_profile_t>(profile_id));
}

std::size_t srtp_key_info_t::get_salt_length(srtp_profile_id_t profile_id)
{
    return srtp_profile_get_master_salt_length(static_cast<srtp_profile_t>(profile_id));
}

std::size_t srtp_key_info_t::get_master_length(srtp_profile_id_t profile_id)
{
    return get_key_length(profile_id) + get_salt_length(profile_id);
}

srtp_key_info_t::srtp_key_info_t(srtp_profile_id_t profile_id
                                 , const srtp_key_t &key)
    : profile_id(profile_id)
    , key(key)
{

}

srtp_key_info_t::srtp_key_info_t(srtp_profile_id_t profile_id
                                 , srtp_key_t &&key)
    : profile_id(profile_id)
    , key(std::move(key))
{

}

bool srtp_key_info_t::operator ==(const srtp_key_info_t &other) const
{
    return profile_id == other.profile_id
            && key == other.key;
}

bool srtp_key_info_t::operator !=(const srtp_key_info_t &other) const
{
    return ! operator == (other);
}

bool srtp_key_info_t::is_valid() const
{
    return profile_id != srtp_profile_id_t::none;
}

std::size_t srtp_key_info_t::key_length() const
{
    return get_key_length(profile_id);
}

std::size_t srtp_key_info_t::salt_length() const
{
    return get_salt_length(profile_id);
}

std::size_t srtp_key_info_t::master_length() const
{
    return get_master_length(profile_id);
}

}
