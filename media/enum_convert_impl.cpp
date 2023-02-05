#include "core/enum_converter_defs.h"

#include "device_types.h"
#include "media_types.h"
#include "audio_types.h"
#include "video_types.h"

namespace mpl::core::utils
{

using namespace media;

declare_enum_converter_begin(device_type_t)
    declare_pair(device_type_t, undefined),
    declare_pair(device_type_t, v4l2_in),
    declare_pair(device_type_t, v4l2_out),
    declare_pair(device_type_t, libav_in),
    declare_pair(device_type_t, libav_out),
    declare_pair(device_type_t, vnc),
    declare_pair(device_type_t, custom)
declare_enum_converter_end(device_type_t)

declare_enum_converter_begin(media_type_t)
    declare_pair(media_type_t, undefined),
    declare_pair(media_type_t, audio),
    declare_pair(media_type_t, video),
    declare_pair(media_type_t, data),
    declare_pair(media_type_t, custom),
declare_enum_converter_end(media_type_t)

declare_enum_converter_begin(audio_format_id_t)
    declare_pair(audio_format_id_t, undefined),
    declare_pair(audio_format_id_t, pcm8),
    declare_pair(audio_format_id_t, pcm16),
    declare_pair(audio_format_id_t, pcm32),
    declare_pair(audio_format_id_t, float32),
    declare_pair(audio_format_id_t, float64),
    declare_pair(audio_format_id_t, pcm8p),
    declare_pair(audio_format_id_t, pcm16p),
    declare_pair(audio_format_id_t, pcm32p),
    declare_pair(audio_format_id_t, float32p),
    declare_pair(audio_format_id_t, float64p),
    declare_pair(audio_format_id_t, pcma),
    declare_pair(audio_format_id_t, pcmu),
    declare_pair(audio_format_id_t, opus),
    declare_pair(audio_format_id_t, aac)
declare_enum_converter_end(audio_format_id_t)

declare_enum_converter_begin(video_format_id_t)
    declare_pair(video_format_id_t, undefined),
    declare_pair(video_format_id_t, yuv420p),
    declare_pair(video_format_id_t, yuv422p),
    declare_pair(video_format_id_t, yuv444p),
    declare_pair(video_format_id_t, yuv411p),
    declare_pair(video_format_id_t, yuyv),
    declare_pair(video_format_id_t, uyvy),
    declare_pair(video_format_id_t, yuv410),
    declare_pair(video_format_id_t, nv12),
    declare_pair(video_format_id_t, nv21),
    declare_pair(video_format_id_t, nv16),
    declare_pair(video_format_id_t, bgr555),
    declare_pair(video_format_id_t, bgr555x),
    declare_pair(video_format_id_t, bgr565),
    declare_pair(video_format_id_t, bgr565x),
    declare_pair(video_format_id_t, rgb555),
    declare_pair(video_format_id_t, rgb555x),
    declare_pair(video_format_id_t, rgb565),
    declare_pair(video_format_id_t, rgb565x),
    declare_pair(video_format_id_t, bgr8),
    declare_pair(video_format_id_t, rgb8),
    declare_pair(video_format_id_t, bgr24),
    declare_pair(video_format_id_t, rgb24),
    declare_pair(video_format_id_t, bgr32),
    declare_pair(video_format_id_t, rgb32),
    declare_pair(video_format_id_t, abgr32),
    declare_pair(video_format_id_t, argb32),
    declare_pair(video_format_id_t, bgra32),
    declare_pair(video_format_id_t, rgba32),
    declare_pair(video_format_id_t, gray8),
    declare_pair(video_format_id_t, gray16),
    declare_pair(video_format_id_t, gray16x),
    declare_pair(video_format_id_t, sbggr8),
    declare_pair(video_format_id_t, sgbrg8),
    declare_pair(video_format_id_t, sgrbg8),
    declare_pair(video_format_id_t, srggb8),
    declare_pair(video_format_id_t, png),
    declare_pair(video_format_id_t, jpeg),
    declare_pair(video_format_id_t, mjpeg),
    declare_pair(video_format_id_t, gif),
    declare_pair(video_format_id_t, h265),
    declare_pair(video_format_id_t, h264),
    declare_pair(video_format_id_t, h263),
    declare_pair(video_format_id_t, h263p),
    declare_pair(video_format_id_t, h261),
    declare_pair(video_format_id_t, vp8),
    declare_pair(video_format_id_t, vp9),
    declare_pair(video_format_id_t, mpeg4),
    declare_pair(video_format_id_t, cpia)
declare_enum_converter_end(video_format_id_t)

}
