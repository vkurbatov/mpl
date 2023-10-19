#include "tls_config.h"

namespace mpl::net
{

std::string tls_config_t::default_chipers = "DEFAULT:!NULL:!aNULL:!SHA256:!SHA384:!aECDH:!AESGCM+AES256:!aPSK";

tls_config_t::tls_config_t(tls_method_t method
                           , timestamp_t timeout
                           , std::size_t retry_count
                           , std::size_t mtu_size
                           , bool srtp_enable
                           , const std::string_view &chipers)
    : method(method)
    , timeout(timeout)
    , retry_count(retry_count)
    , mtu_size(mtu_size)
    , srtp_enable(srtp_enable)
    , chipers(chipers)
{

}


}
