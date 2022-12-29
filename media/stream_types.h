#ifndef MPL_STREAM_TYPES_H
#define MPL_STREAM_TYPES_H

#include <cstdint>

namespace mpl
{
    enum class stream_direction_t
    {
        undefined = -1,
        input,
        output
    };

    using stream_id_t = std::int32_t;
    constexpr stream_id_t stream_id_undefined = -1;
}

#endif // MPL_STREAM_TYPES_H
