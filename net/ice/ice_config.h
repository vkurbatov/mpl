#ifndef MPL_NET_ICE_CONFIG_H
#define MPL_NET_ICE_CONFIG_H

#include "core/time_types.h"
#include "ice_server_params.h"

namespace mpl::net
{

struct ice_config_t
{
    static constexpr std::size_t default_retry_count = 3;
    static constexpr timestamp_t default_allocate_timeout = durations::milliseconds(1000);
    static constexpr timestamp_t default_ice_wait_timeout = durations::milliseconds(500);
    static constexpr timestamp_t default_ice_timeout = durations::milliseconds(500);
    static constexpr timestamp_t default_ice_check_interval = durations::milliseconds(1000);

    ice_server_params_t::array_t    ice_servers;
    std::size_t                     retry_count;
    timestamp_t                     allocate_timeout;
    timestamp_t                     ice_wait_timeout;
    timestamp_t                     ice_timeout;
    timestamp_t                     ice_check_interval;

    ice_config_t(const ice_server_params_t::array_t& ice_servers = {}
                , std::size_t retry_count = default_retry_count
                , timestamp_t allocate_timeout = default_allocate_timeout
                , timestamp_t ice_wait_timeout = default_ice_wait_timeout
                , timestamp_t ice_timeout = default_ice_timeout
                , timestamp_t ice_check_interval = default_ice_check_interval);
};

}

#endif // MPL_NET_ICE_CONFIG_H
