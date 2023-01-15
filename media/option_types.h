#ifndef MPL_OPTION_TYPES_H
#define MPL_OPTION_TYPES_H

#include <cstdint>
#include "tools/base/any_base.h"

namespace mpl
{

using option_id_t = std::int64_t;
using option_value_t = base::any;

constexpr option_id_t opt_reserved_base = 0x0000000;
constexpr option_id_t opt_fmt_base      = 0x0001000;
constexpr option_id_t opt_codec_base    = 0x0002000;
constexpr option_id_t opt_other_base    = 0x0010000;


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

constexpr option_id_t user_base = 0x10000000;


}

#endif // MPL_OPTION_TYPES_H
