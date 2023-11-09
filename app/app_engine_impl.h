#ifndef MPL_APP_ENGINE_IMPL_H
#define MPL_APP_ENGINE_IMPL_H

#include "i_app_engine.h"

namespace mpl::app
{

struct app_config_t;

class app_engine_factory
{
public:
    static app_engine_factory& get_instance();
    i_app_engine::u_ptr_t create_engine(const app_config_t& config);
};

}

#endif // MPL_APP_ENGINE_IMPL_H
