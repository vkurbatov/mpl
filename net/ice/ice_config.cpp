#include "ice_config.h"

namespace mpl::net
{

ice_config_t::ice_config_t(const ice_server_params_t::array_t& ice_servers
                           , std::size_t retry_count
                           , timestamp_t ice_timeout)
    : ice_servers(ice_servers)
    , retry_count(retry_count)
    , ice_timeout(ice_timeout)
{

}

timestamp_t ice_config_t::ice_check_interval() const
{
    return ((retry_count + 2) * ice_timeout);
}

}
