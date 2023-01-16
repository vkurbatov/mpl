#ifndef MPL_MEDIA_OPTION_TYPES_H
#define MPL_MEDIA_OPTION_TYPES_H

#include "core/option_types.h"

namespace mpl
{

constexpr option_id_t opt_media_base    = opt_other_base;
constexpr option_id_t opt_fmt_base      = opt_media_base;
constexpr option_id_t opt_codec_base    = opt_fmt_base + 0x00010000;

constexpr option_id_t opt_fmt_stream_id =       opt_fmt_base + 0;
constexpr option_id_t opt_fmt_device_id =       opt_fmt_base + 1;

constexpr option_id_t opt_codec_name =          opt_codec_base + 0;
constexpr option_id_t opt_codec_bitrate =       opt_codec_base + 1;
constexpr option_id_t opt_codec_gop =           opt_codec_base + 2;
constexpr option_id_t opt_codec_frame_size =    opt_codec_base + 3;
constexpr option_id_t opt_codec_profile =       opt_codec_base + 4;
constexpr option_id_t opt_codec_level =         opt_codec_base + 5;
constexpr option_id_t opt_codec_extra_data =    opt_codec_base + 6;
constexpr option_id_t opt_codec_params =        opt_codec_base + 7;

}

#endif // MPL_MEDIA_OPTION_TYPES_H

