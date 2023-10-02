#ifndef MPL_TIME_UTILS_H
#define MPL_TIME_UTILS_H

#include "core/time_types.h"
#include <string>

namespace mpl::utils::time
{

std::string to_string(timestamp_t timestamp = timestamp_null);
timestamp_t now(timestamp_t duration = durations::nanosecond);
void sleep(timestamp_t wait_time, bool clock_align = false);
timestamp_t get_ticks(timestamp_t duration = durations::nanosecond);

}

#endif // MPL_TIME_UTILS_H
