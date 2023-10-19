#ifndef MPL_NET_TLS_CONFIG_H
#define MPL_NET_TLS_CONFIG_H

#include "core/time_types.h"
#include "tls_types.h"
#include <string>

namespace mpl::net
{

struct tls_config_t
{
    static std::string default_chipers;
    constexpr static timestamp_t default_timeout = durations::milliseconds(1000);
    constexpr static std::size_t default_retry_count = 3;
    constexpr static std::size_t default_mtu_size = 1500;

    tls_method_t    method;
    timestamp_t     timeout;
    std::size_t     retry_count;
    std::size_t     mtu_size;

    bool            srtp_enable;
    std::string     chipers;


    tls_config_t(tls_method_t method = tls_method_t::dtls
                 , timestamp_t timeout = default_timeout
                 , std::size_t retry_count = default_retry_count
                 , std::size_t mtu_size = default_mtu_size
                 , bool srtp_enable = false
                 , const std::string_view& chipers = default_chipers);
};

}

#endif // MPL_NET_TLS_CONFIG_H
