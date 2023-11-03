#include "utils/enum_converter_defs.h"

#include "device_types.h"
#include "media_types.h"
#include "audio_types.h"
#include "video_types.h"

#include "tools/wap/wap_base.h"

namespace mpl::utils
{

using namespace media;

__declare_enum_converter_begin(device_type_t)
    __declare_enum_pair(device_type_t, undefined),
    __declare_enum_pair(device_type_t, v4l2_in),
    __declare_enum_pair(device_type_t, v4l2_out),
    __declare_enum_pair(device_type_t, libav_in),
    __declare_enum_pair(device_type_t, libav_out),
    __declare_enum_pair(device_type_t, vnc),
    __declare_enum_pair(device_type_t, ipc_in),
    __declare_enum_pair(device_type_t, ipc_out),
    __declare_enum_pair(device_type_t, apm),
    __declare_enum_pair(device_type_t, visca),
    __declare_enum_pair(device_type_t, custom)
__declare_enum_converter_end(device_type_t)

__declare_enum_converter_begin(media_type_t)
    __declare_enum_pair(media_type_t, undefined),
    __declare_enum_pair(media_type_t, audio),
    __declare_enum_pair(media_type_t, video),
    __declare_enum_pair(media_type_t, data),
    __declare_enum_pair(media_type_t, custom),
__declare_enum_converter_end(media_type_t)

__declare_enum_converter_begin(audio_format_id_t)
    __declare_enum_pair(audio_format_id_t, undefined),
    __declare_enum_pair(audio_format_id_t, pcm8),
    __declare_enum_pair(audio_format_id_t, pcm16),
    __declare_enum_pair(audio_format_id_t, pcm32),
    __declare_enum_pair(audio_format_id_t, float32),
    __declare_enum_pair(audio_format_id_t, float64),
    __declare_enum_pair(audio_format_id_t, pcm8p),
    __declare_enum_pair(audio_format_id_t, pcm16p),
    __declare_enum_pair(audio_format_id_t, pcm32p),
    __declare_enum_pair(audio_format_id_t, float32p),
    __declare_enum_pair(audio_format_id_t, float64p),
    __declare_enum_pair(audio_format_id_t, pcma),
    __declare_enum_pair(audio_format_id_t, pcmu),
    __declare_enum_pair(audio_format_id_t, opus),
    __declare_enum_pair(audio_format_id_t, aac)
__declare_enum_converter_end(audio_format_id_t)

__declare_enum_converter_begin(video_format_id_t)
    __declare_enum_pair(video_format_id_t, undefined),
    __declare_enum_pair(video_format_id_t, yuv420p),
    __declare_enum_pair(video_format_id_t, yuv422p),
    __declare_enum_pair(video_format_id_t, yuv444p),
    __declare_enum_pair(video_format_id_t, yuv411p),
    __declare_enum_pair(video_format_id_t, yuyv),
    __declare_enum_pair(video_format_id_t, uyvy),
    __declare_enum_pair(video_format_id_t, yuv410),
    __declare_enum_pair(video_format_id_t, nv12),
    __declare_enum_pair(video_format_id_t, nv21),
    __declare_enum_pair(video_format_id_t, nv16),
    __declare_enum_pair(video_format_id_t, bgr555),
    __declare_enum_pair(video_format_id_t, bgr555x),
    __declare_enum_pair(video_format_id_t, bgr565),
    __declare_enum_pair(video_format_id_t, bgr565x),
    __declare_enum_pair(video_format_id_t, rgb555),
    __declare_enum_pair(video_format_id_t, rgb555x),
    __declare_enum_pair(video_format_id_t, rgb565),
    __declare_enum_pair(video_format_id_t, rgb565x),
    __declare_enum_pair(video_format_id_t, bgr8),
    __declare_enum_pair(video_format_id_t, rgb8),
    __declare_enum_pair(video_format_id_t, bgr24),
    __declare_enum_pair(video_format_id_t, rgb24),
    __declare_enum_pair(video_format_id_t, bgr32),
    __declare_enum_pair(video_format_id_t, rgb32),
    __declare_enum_pair(video_format_id_t, abgr32),
    __declare_enum_pair(video_format_id_t, argb32),
    __declare_enum_pair(video_format_id_t, bgra32),
    __declare_enum_pair(video_format_id_t, rgba32),
    __declare_enum_pair(video_format_id_t, gray8),
    __declare_enum_pair(video_format_id_t, gray16),
    __declare_enum_pair(video_format_id_t, gray16x),
    __declare_enum_pair(video_format_id_t, sbggr8),
    __declare_enum_pair(video_format_id_t, sgbrg8),
    __declare_enum_pair(video_format_id_t, sgrbg8),
    __declare_enum_pair(video_format_id_t, srggb8),
    __declare_enum_pair(video_format_id_t, png),
    __declare_enum_pair(video_format_id_t, jpeg),
    __declare_enum_pair(video_format_id_t, mjpeg),
    __declare_enum_pair(video_format_id_t, gif),
    __declare_enum_pair(video_format_id_t, h265),
    __declare_enum_pair(video_format_id_t, h264),
    __declare_enum_pair(video_format_id_t, h263),
    __declare_enum_pair(video_format_id_t, h263p),
    __declare_enum_pair(video_format_id_t, h261),
    __declare_enum_pair(video_format_id_t, vp8),
    __declare_enum_pair(video_format_id_t, vp9),
    __declare_enum_pair(video_format_id_t, mpeg4),
    __declare_enum_pair(video_format_id_t, cpia)
__declare_enum_converter_end(video_format_id_t)

__declare_enum_converter_begin(pt::wap::echo_cancellation_mode_t)
    __declare_enum_pair(pt::wap::echo_cancellation_mode_t, none),
    __declare_enum_pair(pt::wap::echo_cancellation_mode_t, low),
    __declare_enum_pair(pt::wap::echo_cancellation_mode_t, moderation),
    __declare_enum_pair(pt::wap::echo_cancellation_mode_t, high)
__declare_enum_converter_end(pt::wap::echo_cancellation_mode_t)

__declare_enum_converter_begin(pt::wap::gain_control_mode_t)
    __declare_enum_pair(pt::wap::gain_control_mode_t, none),
    __declare_enum_pair(pt::wap::gain_control_mode_t, adaptive_analog),
    __declare_enum_pair(pt::wap::gain_control_mode_t, adaptive_digital),
    __declare_enum_pair(pt::wap::gain_control_mode_t, fixed_digital)
__declare_enum_converter_end(pt::wap::gain_control_mode_t)

__declare_enum_converter_begin(pt::wap::noise_suppression_mode_t)
    __declare_enum_pair(pt::wap::noise_suppression_mode_t, none),
    __declare_enum_pair(pt::wap::noise_suppression_mode_t, low),
    __declare_enum_pair(pt::wap::noise_suppression_mode_t, moderate),
    __declare_enum_pair(pt::wap::noise_suppression_mode_t, high),
    __declare_enum_pair(pt::wap::noise_suppression_mode_t, very_high)
__declare_enum_converter_end(pt::wap::noise_suppression_mode_t)

__declare_enum_converter_begin(pt::wap::voice_detection_mode_t)
    __declare_enum_pair(pt::wap::voice_detection_mode_t, none),
    __declare_enum_pair(pt::wap::voice_detection_mode_t, very_low),
    __declare_enum_pair(pt::wap::voice_detection_mode_t, low),
    __declare_enum_pair(pt::wap::voice_detection_mode_t, moderate),
    __declare_enum_pair(pt::wap::voice_detection_mode_t, high)
__declare_enum_converter_end(pt::wap::voice_detection_mode_t)

}
