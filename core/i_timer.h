#ifndef MPL_I_TIMER_H
#define MPL_I_TIMER_H

#include "time_types.h"
#include <memory>

namespace mpl
{

class i_timer
{
public:
    using timer_id_t = std::uint32_t;
    using u_ptr_t = std::unique_ptr<i_timer>;
    using s_ptr_t = std::unique_ptr<i_timer>;

    virtual ~i_timer() = default;
    virtual timer_id_t id() const = 0;
    virtual bool start(timestamp_t timeout) = 0;
    virtual bool stop() = 0;
    virtual bool processed() const = 0;
    virtual timestamp_t target_time() const = 0;

};

}

#endif // MPL_I_TIMER_H
