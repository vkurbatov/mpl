#ifndef MPL_I_TIMER_MANAGER_H
#define MPL_I_TIMER_MANAGER_H

#include "i_timer.h"
#include <functional>

namespace mpl
{

class i_timer_manager
{
public:
    using timer_handler_t = std::function<void()>;
    using u_ptr_t = std::unique_ptr<i_timer_manager>;
    using s_ptr_t = std::shared_ptr<i_timer_manager>;
    using w_ptr_t = std::weak_ptr<i_timer_manager>;

    virtual ~i_timer_manager() = default;
    virtual i_timer::u_ptr_t create_timer(const timer_handler_t& timer_handler) = 0;
    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual bool is_started() const = 0;

};

}

#endif // MPL_I_TIMER_MANAGER_H
