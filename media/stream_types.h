#ifndef MPL_MEDIA_STREAM_TYPES_H
#define MPL_MEDIA_STREAM_TYPES_H

#include <cstdint>

namespace mpl::media
{
    enum class stream_direction_t
    {
        undefined = 0,
        input,
        output
    };

    using stream_id_t = std::int32_t;
    constexpr stream_id_t stream_id_undefined = 0;
}

#endif // MPL_MEDIA_STREAM_TYPES_H
