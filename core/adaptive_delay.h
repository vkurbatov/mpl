#ifndef MPL_ADAPTIVE_DELAY_H
#define MPL_ADAPTIVE_DELAY_H

#include "time_types.h"

namespace mpl
{

class adaptive_delay
{
    static constexpr std::size_t default_max_drift_periods = 3;

    timestamp_t     m_target_time;

public:
    adaptive_delay();

    void reset(timestamp_t delay = 0);

    void wait(timestamp_t delay
              , std::size_t max_drift_periods = default_max_drift_periods);
};

}

#endif // MPL_ADAPTIVE_DELAY_H
