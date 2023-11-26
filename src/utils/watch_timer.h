#ifndef MPL_UTILS_WATCH_TIMER_H
#define MPL_UTILS_WATCH_TIMER_H

#include "core/time_types.h"

namespace mpl
{

class watch_timer
{
    timestamp_t     m_time_point;
    timestamp_t     m_interval;
public:
    watch_timer(timestamp_t interval);

    void reset();
    void reset(timestamp_t interval);
    bool is_timeout() const;
    bool check_timeout();
    timestamp_t target_time() const;
    timestamp_t interval() const;
};

}

#endif // MPL_UTILS_WATCH_TIMER_H
