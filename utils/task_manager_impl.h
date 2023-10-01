#ifndef MPL_TASK_MANAGER_IMPL_H
#define MPL_TASK_MANAGER_IMPL_H

#include "core/time_types.h"
#include "core/i_task_manager.h"

namespace mpl
{

class task_manager_factory
{
public:
    struct config_t
    {
        std::uint32_t   max_workers = 0; // auto
        timestamp_t     idle_time = timestamp_infinite;
        std::uint32_t   max_queued_workers = 0; // infinite
    };

    static task_manager_factory& get_instance();
    static i_task_manager& singe_manager();

    i_task_manager::u_ptr_t create_manager(const config_t& config);
};

}

#endif // MPL_TASK_MANAGER_IMPL_H
