#ifndef MPL_MEDIA_TYPES_H
#define MPL_MEDIA_TYPES_H

#include "core/message_types.h"
#include <cstdint>

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

enum class media_data_type_t
{
    undefined,
    frame
};


enum class video_frame_type_t
{
    undefined = 0,
    delta_frame,
    key_frame
};

using frame_id_t = std::int32_t;
using stream_id_t = std::int32_t;
using track_id_t = std::int32_t;
using layer_id_t = std::int32_t;

constexpr frame_id_t frame_id_undefined = -1;
constexpr stream_id_t stream_id_undefined = -1;
constexpr track_id_t track_id_undefined = -1;
constexpr layer_id_t layer_id_undefined = -1;

constexpr stream_id_t default_stream_id = 0;
constexpr track_id_t default_audio_track_id = 0;
constexpr track_id_t default_video_track_id = 1;
constexpr track_id_t default_data_track_id = 2;
constexpr layer_id_t default_layer_id = 0;

constexpr std::uint32_t video_sample_rate = 90000;
constexpr std::int64_t media_buffer_index = 0;
constexpr std::int64_t extension_data_index = 1;


}

#endif // MPL_MEDIA_TYPES_H
