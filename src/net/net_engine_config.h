#ifndef MPL_NET_ENGINE_CONFIG_H
#define MPL_NET_ENGINE_CONFIG_H

#include "tls/tls_config.h"
#include "ice/ice_config.h"


namespace mpl::net
{

struct net_engine_config_t
{
    static constexpr std::size_t default_listen_workers = 1;

    std::size_t     listen_workers;
    ice_config_t    ice_config;
    tls_config_t    tls_config;

    net_engine_config_t(std::size_t listen_workers = default_listen_workers
                        , const ice_config_t& ice_config = {}
                        , const tls_config_t& tls_config = {});
};

}

#endif // MPL_NET_ENGINE_CONFIG_H
