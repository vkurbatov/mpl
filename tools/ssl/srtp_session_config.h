#ifndef SSL_SRTP_SESSION_CONFIG_H
#define SSL_SRTP_SESSION_CONFIG_H

#include "srtp_key_info.h"

namespace ssl
{

struct srtp_session_config_t
{
    srtp_direction_t        direction;
    srtp_key_info_t         key_info;
    srtp_session_config_t(srtp_direction_t direction = srtp_direction_t::inbound
                          , const srtp_key_info_t& key_info = {});

    bool is_valid() const;
};

}

#endif // SSL_SRTP_SESSION_CONFIG_H
