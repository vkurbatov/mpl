#ifndef MPL_UTILS_CORE_ENGINE_IMPL_H
#define MPL_UTILS_CORE_ENGINE_IMPL_H

#include "core/i_core_engine.h"

namespace mpl
{

struct core_engine_config_t;

class core_engine_factory
{
public:
    static core_engine_factory& get_instance();
    i_core_engine::u_ptr_t create_engine(const core_engine_config_t& config);
};

}

#endif // MPL_UTILS_CORE_ENGINE_IMPL_H
