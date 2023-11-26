#include "app_config.h"

namespace mpl::app
{

app_config_t::app_config_t(const core_engine_config_t &core_config
                           , const net::net_engine_config_t &net_config
                           , const media::media_engine_config_t &media_config)
    : core_config(core_config)
    , net_config(net_config)
    , media_config(media_config)
{

}



}
