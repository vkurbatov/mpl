#ifndef MPL_MEDIA_ENGINE_CONFIG_H
#define MPL_MEDIA_ENGINE_CONFIG_H

#include <string>

namespace mpl::media
{

struct media_engine_config_t
{
    std::string     ipc_name;
    std::size_t     ipc_size;

    media_engine_config_t(const std::string_view& ipc_name = {}
                          , std::size_t ipc_size = 0);
};

}

#endif // MPL_MEDIA_ENGINE_CONFIG_H
