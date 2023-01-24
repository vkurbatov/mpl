#ifndef MPL_MEDIA_TYPES_H
#define MPL_MEDIA_TYPES_H

#include <cstdint>

namespace mpl::media
{

enum class media_type_t
{
    undefined = -1,
    audio,
    video,
    data,
    custom
};

enum class transcoder_type_t
{
    undefined = -1,
    encoder,
    transcoder,
    converter
};

using frame_id_t = std::int32_t;

constexpr frame_id_t frame_id_undefined = -1;
constexpr std::uint32_t video_sample_rate = 90000;
constexpr std::int64_t main_media_buffer_index = 0;

}

#endif // MPL_MEDIA_TYPES_H
