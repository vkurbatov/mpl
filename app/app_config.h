#ifndef MPL_APP_CONFIG_H
#define MPL_APP_CONFIG_H

#include "core/core_engine_config.h"
#include "media/media_engine_config.h"
#include "net/net_engine_config.h"

namespace mpl::app
{

struct app_config_t
{
    core_engine_config_t            core_config;
    net::net_engine_config_t        net_config;
    media::media_engine_config_t    media_config;

    app_config_t(const core_engine_config_t& core_config = {}
                , const net::net_engine_config_t& net_config = {}
                , const media::media_engine_config_t& media_config = {});



};

}

#endif // MPL_APP_CONFIG_H
