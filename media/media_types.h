#ifndef MPL_MEDIA_TYPES_H
#define MPL_MEDIA_TYPES_H

#include "core/message_types.h"

namespace mpl::media
{

enum class media_type_t
{
    undefined = 0,
    audio,
    video,
    data,
    custom
};

enum class transcoder_type_t
{
    undefined = 0,
    encoder,
    transcoder,
    converter
};

using frame_id_t = std::int32_t;
using stream_id_t = std::int32_t;

constexpr frame_id_t frame_id_undefined = 0;
constexpr stream_id_t stream_id_undefined = 0;
constexpr std::uint32_t video_sample_rate = 90000;
constexpr std::int64_t media_buffer_index = 0;
constexpr std::int64_t extension_data_index = 1;

constexpr message_subtype_t message_subtype_media_base = message_subtype_core_base + message_subtype_range;
constexpr message_subtype_t message_subtype_media_frame = message_subtype_media_base + 1;

}

#endif // MPL_MEDIA_TYPES_H
