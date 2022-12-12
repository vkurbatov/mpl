#include "time_base.h"

#include <chrono>
#include <thread>

namespace base
{

adaptive_timer_t::adaptive_timer_t(std::uint64_t tick_size)
    : tick_size(tick_size)
{
    reset();
}

uint64_t adaptive_timer_t::now(std::uint32_t tick_size)
{
    return tick_size * std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() / std::nano::den;
}

void adaptive_timer_t::reset()
{
    time_base = now(tick_size);
}

bool adaptive_timer_t::wait(std::uint64_t wait_time
                            , bool is_wait)
{
    auto elapsed_time = elapsed();

    if (elapsed_time >= wait_time)
    {
        time_base += wait_time;
        return true;
    }

    if (is_wait && tick_size > 0)
    {
        std::this_thread::sleep_for(std::nano::den * std::chrono::nanoseconds(wait_time - elapsed_time) / tick_size);
        time_base += wait_time;
        return true;
    }

    return false;
}

uint64_t adaptive_timer_t::elapsed() const
{
    return now(tick_size) - time_base;
}

}
