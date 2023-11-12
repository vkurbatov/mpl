#include "net_engine_config.h"

namespace mpl::net
{

net_engine_config_t::net_engine_config_t(std::size_t listen_workers
                                         , const ice_config_t &ice_config
                                         , const tls_config_t &tls_config)
    : listen_workers(listen_workers)
    , ice_config(ice_config)
    , tls_config(tls_config)
{

}

}
