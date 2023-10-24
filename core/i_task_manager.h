#ifndef MPL_I_TASK_MANAGER_H
#define MPL_I_TASK_MANAGER_H

#include "i_task.h"
#include <functional>
namespace mpl
{

class i_task_manager
{
public:

    using task_handler_t = std::function<void()>;
    using u_ptr_t = std::unique_ptr<i_task_manager>;
    using s_ptr_t = std::shared_ptr<i_task_manager>;

    virtual i_task::s_ptr_t add_task(const task_handler_t& task_handler) = 0;
    // virtual bool remove_task(task_id_t task_id) = 0;
    virtual void reset() = 0;
    virtual std::size_t pending_tasks() const = 0;
    virtual std::size_t active_workers() const = 0;
    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual bool is_started() const = 0;
};

}

#endif // MPL_I_TASK_MANAGER_H
