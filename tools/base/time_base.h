#ifndef TIME_BASE_H
#define TIME_BASE_H

#include <cstdint>

namespace base
{

struct adaptive_timer_t
{
    std::uint64_t   tick_size;
    std::uint64_t   time_base;
    adaptive_timer_t(std::uint64_t tick_size = 1000);

    static std::uint64_t now(std::uint32_t tick_size = 1000);

    void reset();
    bool wait(std::uint64_t wait_time
              , bool is_wait = true);
    std::uint64_t elapsed() const;
};

}

#endif // TIMER_BASE_H
