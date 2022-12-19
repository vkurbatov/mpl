#ifndef MPL_MEDIA_TYPES_H
#define MPL_MEDIA_TYPES_H

#include <cstdint>

namespace mpl
{

enum class media_type_t
{
    undefined = -1,
    audio,
    video,
    data,
    custom
};

using stream_id_t = std::int32_t;
using frame_id_t = std::int32_t;

constexpr stream_id_t stream_id_undefined = -1;
constexpr frame_id_t frame_id_undefined = -1;

}

#endif // MPL_MEDIA_TYPES_H
