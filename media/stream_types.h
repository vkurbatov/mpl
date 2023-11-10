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
}

#endif // MPL_MEDIA_STREAM_TYPES_H
