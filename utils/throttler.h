#ifndef MPL_UTILS_THROTTLER_H
#define MPL_UTILS_THROTTLER_H

#include "core/time_types.h"

namespace mpl
{

class throttler
{
    timestamp_t     m_throttle_time;
    std::uint32_t   m_counter;
    timestamp_t     m_next_time;
    bool            m_has_set;

public:

    throttler(timestamp_t throttle_time);

    void set();
    bool get();

    std::size_t completed() const;
    bool has_set() const;
    bool has_get() const;

    void reset();
    void reset(timestamp_t throttle_time);
};

}
#endif // MPL_UTILS_THROTTLER_H
