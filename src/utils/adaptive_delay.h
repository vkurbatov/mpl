#ifndef MPL_UTILS_ADAPTIVE_DELAY_H
#define MPL_UTILS_ADAPTIVE_DELAY_H

#include "core/time_types.h"

namespace mpl
{

class adaptive_delay
{
    timestamp_t     m_last_time;

public:
    static timestamp_t now();
    adaptive_delay();
    timestamp_t elapsed() const;
    void wait(timestamp_t wait_period);
    void reset();
};

}

#endif // MPL_UTILS_ADAPTIVE_DELAY_H
