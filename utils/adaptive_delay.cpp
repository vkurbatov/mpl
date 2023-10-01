#include "adaptive_delay.h"
#include "utils/time_utils.h"

namespace mpl
{

adaptive_delay::adaptive_delay()
{
    reset();
}

void adaptive_delay::reset(timestamp_t delay)
{
    m_target_time = mpl::core::utils::get_ticks() + delay;
}

void adaptive_delay::wait(timestamp_t delay
                          , std::size_t max_drift_periods)
{
    auto now = mpl::core::utils::get_ticks();

    timestamp_t max_drift = delay * max_drift_periods;

    auto dt = m_target_time - now;

    if (max_drift != 0
            && std::abs(dt) > max_drift)
    {
        m_target_time = now + delay;
        dt = delay / 2;
    }
    else
    {
        m_target_time += delay;

        if (dt < 0)
        {
            dt = 0;
        }
    }

    mpl::core::utils::sleep(dt);
}

}
