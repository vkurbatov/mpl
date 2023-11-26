#include "adaptive_delay.h"
#include "utils/time_utils.h"

namespace mpl
{

timestamp_t adaptive_delay::now()
{
    return  utils::time::get_ticks();
}

adaptive_delay::adaptive_delay()
    : m_last_time(now())
{

}

timestamp_t adaptive_delay::elapsed() const
{
    return now() - m_last_time;
}

void adaptive_delay::wait(timestamp_t wait_period)
{
    m_last_time = m_last_time + wait_period;
    auto now_tp = now();
    if (m_last_time > now_tp)
    {
        utils::time::sleep(m_last_time - now_tp);
    }
}

void adaptive_delay::reset()
{
    m_last_time = now();
}

}
