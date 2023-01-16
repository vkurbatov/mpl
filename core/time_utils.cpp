#include "time_utils.h"

#include <chrono>
#include <thread>

namespace mpl::utils
{

std::string to_string(timestamp_t timestamp)
{
    static const char* log_time_format  = "%02d.%02d.%02d %02d:%02d:%02d.%06d";

    auto time_point = timestamp == timestamp_null
            ? std::chrono::system_clock::now()
            : std::chrono::system_clock::time_point(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::nanoseconds(timestamp)));

    if (timestamp == timestamp_null)
    {
        timestamp = time_point.time_since_epoch().count();
    }

    auto c_time = std::chrono::system_clock::to_time_t(time_point);

    auto microseconds = (timestamp / 1000) % 1000000;

    auto time_struct = std::localtime(&c_time);

    char time_buff[32];

    std::sprintf(time_buff
                 , log_time_format
                 , time_struct->tm_mday
                 , time_struct->tm_mon + 1
                 , time_struct->tm_year % 100
                 , time_struct->tm_hour
                 , time_struct->tm_min
                 , time_struct->tm_sec
                 , microseconds);

    return time_buff;
}

timestamp_t now(timestamp_t duration)
{
    return duration > 0
            ? std::chrono::high_resolution_clock::now().time_since_epoch().count() / duration
            : std::chrono::high_resolution_clock::now().time_since_epoch().count();
}

void sleep(timestamp_t wait_time, bool clock_align)
{
    if (clock_align)
    {
        auto dt = now() % wait_time;
        if (dt != wait_time)
        {
            wait_time -= dt;
        }
    }
    std::this_thread::sleep_for(std::chrono::nanoseconds(wait_time));
}


}
