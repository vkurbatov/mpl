#include "throttler.h"
#include "time_utils.h"

namespace mpl
{

throttler::throttler(timestamp_t throttle_time)
    : m_throttle_time(throttle_time)
    , m_counter(0)
    , m_next_time(timestamp_null)
    , m_has_set(false)
{

}

void throttler::set()
{
    m_has_set |= true;
}

bool throttler::get()
{
    if (m_has_set)
    {
        auto now = utils::time::get_ticks();
        if (m_next_time <= now)
        {
            m_counter++;
            m_has_set = false;

            m_next_time = now + m_throttle_time;

            return true;
        }
    }

    return false;
}

std::size_t throttler::completed() const
{
    return m_counter;
}

bool throttler::has_set() const
{
    return m_has_set;
}

bool throttler::has_get() const
{
    return has_set()
            && m_next_time <= utils::time::get_ticks();
}

void throttler::reset()
{
    m_counter = 0;
    m_next_time = timestamp_null;
    m_has_set = false;
}

void throttler::reset(timestamp_t throttle_time)
{
    m_throttle_time = throttle_time;
    reset();
}

}
