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

using frame_id_t = std::int32_t;

constexpr frame_id_t frame_id_undefined = -1;
constexpr std::uint32_t video_sample_rate = 90000;

}

#endif // MPL_MEDIA_TYPES_H
