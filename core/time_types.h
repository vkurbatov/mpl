#ifndef MPL_TIME_TYPES_H
#define MPL_TIME_TYPES_H

#include <cstdint>

namespace mpl
{

using timer_id_t = std::int32_t;
const timer_id_t timer_id_none = -1;

using timestamp_t = std::int64_t;
const timestamp_t timestamp_infinite = -1;
const timestamp_t timestamp_null = 0;


namespace durations
{

const timestamp_t nanosecond = 1;
const timestamp_t microsecond = nanosecond * 1000;
const timestamp_t millisecond = microsecond * 1000;
const timestamp_t second = millisecond * 1000;
const timestamp_t minute = second * 60;
const timestamp_t hour = minute * 60;
const timestamp_t day = hour * 24;

constexpr timestamp_t nanoseconds(timestamp_t timestamp) { return timestamp * nanosecond; }
constexpr timestamp_t microseconds(timestamp_t timestamp) { return timestamp * microsecond; }
constexpr timestamp_t milliseconds(timestamp_t timestamp) { return timestamp * millisecond; }
constexpr timestamp_t seconds(timestamp_t timestamp) { return timestamp * second; }
constexpr timestamp_t minutes(timestamp_t timestamp) { return timestamp * minute; }
constexpr timestamp_t hours(timestamp_t timestamp) { return timestamp * hour; }
constexpr timestamp_t days(timestamp_t timestamp) { return timestamp * day; }

constexpr timestamp_t to_nanoseconds(timestamp_t timestamp) { return timestamp / nanosecond; }
constexpr timestamp_t to_microseconds(timestamp_t timestamp) { return timestamp / microsecond; }
constexpr timestamp_t to_milliseconds(timestamp_t timestamp) { return timestamp / millisecond; }
constexpr timestamp_t to_seconds(timestamp_t timestamp) { return timestamp / second; }
constexpr timestamp_t to_minutes(timestamp_t timestamp) { return timestamp / minute; }
constexpr timestamp_t to_hours(timestamp_t timestamp) { return timestamp / hour; }
constexpr timestamp_t to_days(timestamp_t timestamp) { return timestamp / day; }

}

}

#endif // MPL_TIME_TYPES_H
