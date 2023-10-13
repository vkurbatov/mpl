#include "ice_config.h"

namespace mpl::net
{

ice_config_t::ice_config_t(std::size_t retry_count
                           , timestamp_t allocate_timeout
                           , timestamp_t ice_wait_timeout
                           , timestamp_t ice_timeout
                           , timestamp_t ice_check_interval)
    : retry_count(retry_count)
    , allocate_timeout(allocate_timeout)
    , ice_wait_timeout(ice_wait_timeout)
    , ice_timeout(ice_timeout)
    , ice_check_interval(ice_check_interval)
{

}

}
