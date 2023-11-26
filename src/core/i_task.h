#ifndef MPL_I_TASK_H
#define MPL_I_TASK_H

#include <memory>
#include <string>

namespace mpl
{

using task_id_t = std::int64_t;
constexpr task_id_t task_id_none = -1;

class i_task
{
public:

    enum class task_state_t
    {
        ready,
        execute,
        completed,
        cancelled
    };

    using u_ptr_t = std::unique_ptr<i_task>;
    using s_ptr_t = std::shared_ptr<i_task>;

    virtual ~i_task() = default;
    virtual task_id_t task_id() const = 0;
    virtual bool wait() = 0;
    virtual void cancel() = 0;
    virtual task_state_t state() const = 0;
};

}

#endif // MPL_I_TASK_H
