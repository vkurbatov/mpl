#include "srtp_session_config.h"

namespace pt::ssl
{

srtp_session_config_t::srtp_session_config_t(srtp_direction_t direction
                                             , const srtp_key_info_t &key_info)
    : direction(direction)
    , key_info(key_info)
{

}

bool srtp_session_config_t::is_valid() const
{
    return key_info.is_valid();
}


}
