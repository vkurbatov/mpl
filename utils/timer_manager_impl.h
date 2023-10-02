#ifndef MPL_UTILS_TIMER_MANAGER_IMPL_H
#define MPL_UTILS_TIMER_MANAGER_IMPL_H

#include "core/i_timer_manager.h"

namespace mpl
{

class i_task_manager;

class timer_manager_factory
{
public:
    struct config_t
    {

    };

    static timer_manager_factory& get_instance();
    static i_timer_manager& single_manager();

    i_timer_manager::u_ptr_t create_timer_manager(const config_t& config
                                                  , i_task_manager& task_manager);
};


}

#endif // MPL_UTILS_TIMER_MANAGER_IMPL_H
