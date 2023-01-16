#ifndef MPL_TIME_UTILS_H
#define MPL_TIME_UTILS_H

#include "time_types.h"
#include <string>

namespace mpl::core::utils
{

std::string to_string(timestamp_t timestamp = timestamp_null);
timestamp_t now(timestamp_t duration = durations::nanosecond);
void sleep(timestamp_t wait_time, bool clock_align = false);

}

#endif // MPL_TIME_UTILS_H
