#ifndef MPL_CORE_ENGINE_CONFIG_H
#define MPL_CORE_ENGINE_CONFIG_H

#include <cstdint>

namespace mpl
{

struct core_engine_config_t
{
    std::uint32_t     worker_count;
    core_engine_config_t(std::uint32_t worker_count = 0);

    bool operator == (const core_engine_config_t& other) const;
    bool operator != (const core_engine_config_t& other) const;
};

}

#endif // MPL_CORE_ENGINE_CONFIG_H
