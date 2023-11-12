#include "watch_timer.h"
#include "time_utils.h"

namespace mpl
{

watch_timer::watch_timer(timestamp_t interval)
    : m_time_point(utils::time::get_ticks())
    , m_interval(interval)
{

}

void watch_timer::reset()
{
    m_time_point = utils::time::get_ticks();
}

void watch_timer::reset(timestamp_t interval)
{
    m_interval = interval;
    reset();
}

bool watch_timer::is_timeout() const
{
    return utils::time::get_ticks() >= target_time();
}

bool watch_timer::check_timeout()
{
    auto now = utils::time::get_ticks();
    auto dt = now - m_time_point;
    dt -= dt % m_interval;
    if (dt >= m_interval)
    {
        m_time_point += dt;
        return true;
    }

    return false;
}

timestamp_t watch_timer::target_time() const
{
    return m_time_point + m_interval;
}

timestamp_t watch_timer::interval() const
{
    return m_interval;
}


}
