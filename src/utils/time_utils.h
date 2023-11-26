#ifndef MPL_TIME_UTILS_H
#define MPL_TIME_UTILS_H

#include "core/time_types.h"
#include <string>

namespace mpl::utils::time
{

std::string to_string(timestamp_t timestamp = timestamp_null);
timestamp_t zone_offset(timestamp_t duration = durations::nanosecond);
timestamp_t now(timestamp_t duration = durations::nanosecond);
timestamp_t utc_now(timestamp_t duration = durations::nanosecond);
void sleep(timestamp_t wait_time, bool clock_align = false);
timestamp_t get_ticks(timestamp_t duration = durations::nanosecond);
std::uint32_t get_abs_time_24(timestamp_t timestamp);
timestamp_t from_abs_time_24(std::uint32_t abs_time);


}

#endif // MPL_TIME_UTILS_H
