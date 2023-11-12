#ifndef MPL_I_CORE_MODULE_H
#define MPL_I_CORE_MODULE_H

#include "i_module.h"

namespace mpl
{

class i_task_manager;
class i_timer_manager;
class i_buffer_factory;

class i_core_module : public i_module
{
public:
    using u_ptr_t = std::unique_ptr<i_core_module>;
    using s_ptr_t = std::shared_ptr<i_core_module>;

    virtual i_task_manager& task_manager() = 0;
    virtual i_timer_manager& timer_manager() = 0;
    virtual i_buffer_factory& buffer_factory() = 0;
};

}

#endif // MPL_I_CORE_MODULE_H
