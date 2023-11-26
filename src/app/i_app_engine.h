#ifndef MPL_I_APP_ENGINE_H
#define MPL_I_APP_ENGINE_H

#include "core/i_engine.h"
#include "core/i_module.h"

namespace mpl::app
{

class i_app_engine : public i_engine
{
public:
    using u_ptr_t = std::unique_ptr<i_app_engine>;
    using s_ptr_t = std::shared_ptr<i_app_engine>;

    virtual i_module* get_module(module_id_t module_id) = 0;
};


}

#endif // MPL_I_APP_ENGINE_H
