#include "libav_utils.h"
#include "convert_utils.h"
#include <unordered_map>
#include <cstdint>

#include "audio_types.h"
#include "video_types.h"

#include "tools/ffmpeg/libav_base.h"

namespace mpl::utils
{

namespace detail
{

using video_table_t = std::unordered_map<video_format_id_t
                                         , ffmpeg::format_info_t>;

using audio_table_t = std::unordered_map<audio_format_id_t
                                         , ffmpeg::format_info_t>;

static const video_table_t video_format_table =
{
    { video_format_id_t::undefined, { ffmpeg::unknown_pixel_format, ffmpeg::unknown_codec_id } },
    { video_format_id_t::yuv420p,   { ffmpeg::pixel_format_yuv420p, ffmpeg::unknown_codec_id } },
    { video_format_id_t::yuv422p,   { ffmpeg::pixel_format_yuv422p, ffmpeg::unknown_codec_id } },
    { video_format_id_t::yuv444p,   { ffmpeg::pixel_format_yuv444p, ffmpeg::unknown_codec_id } },
    { video_format_id_t::yuv411p,   { ffmpeg::pixel_format_yuv411p, ffmpeg::unknown_codec_id } },
    { video_format_id_t::yuyv,      { ffmpeg::pixel_format_yuyv,    ffmpeg::unknown_codec_id } },
    { video_format_id_t::uyvy,      { ffmpeg::pixel_format_uyvy,    ffmpeg::unknown_codec_id } },
    { video_format_id_t::yuv410,    { ffmpeg::pixel_format_yuv410,  ffmpeg::unknown_codec_id } },
    { video_format_id_t::nv12,      { ffmpeg::pixel_format_nv12,    ffmpeg::unknown_codec_id } },
    { video_format_id_t::nv21,      { ffmpeg::pixel_format_nv21,    ffmpeg::unknown_codec_id } },
    { video_format_id_t::nv16,      { ffmpeg::pixel_format_nv16,    ffmpeg::unknown_codec_id } },
    { video_format_id_t::bgr555,    { ffmpeg::pixel_format_bgr555,  ffmpeg::unknown_codec_id } },
    { video_format_id_t::bgr555x,   { ffmpeg::pixel_format_bgr555x, ffmpeg::unknown_codec_id } },
    { video_format_id_t::bgr565,    { ffmpeg::pixel_format_bgr565,  ffmpeg::unknown_codec_id } },
    { video_format_id_t::bgr565x,   { ffmpeg::pixel_format_bgr565x, ffmpeg::unknown_codec_id } },
    { video_format_id_t::rgb555,    { ffmpeg::pixel_format_rgb555,  ffmpeg::unknown_codec_id } },
    { video_format_id_t::rgb555x,   { ffmpeg::pixel_format_rgb555x, ffmpeg::unknown_codec_id } },
    { video_format_id_t::rgb565,    { ffmpeg::pixel_format_rgb565,  ffmpeg::unknown_codec_id } },
    { video_format_id_t::rgb565x,   { ffmpeg::pixel_format_rgb565x, ffmpeg::unknown_codec_id } },
    { video_format_id_t::bgr8,      { ffmpeg::pixel_format_bgr8,    ffmpeg::unknown_codec_id } },
    { video_format_id_t::rgb8,      { ffmpeg::pixel_format_rgb8,    ffmpeg::unknown_codec_id } },
    { video_format_id_t::bgr24,     { ffmpeg::pixel_format_bgr24,   ffmpeg::unknown_codec_id } },
    { video_format_id_t::rgb24,     { ffmpeg::pixel_format_rgb24,   ffmpeg::unknown_codec_id } },
    { video_format_id_t::bgr32,     { ffmpeg::pixel_format_bgr32,   ffmpeg::unknown_codec_id } },
    { video_format_id_t::rgb32,     { ffmpeg::pixel_format_rgb32,   ffmpeg::unknown_codec_id } },
    { video_format_id_t::abgr32,    { ffmpeg::pixel_format_abgr32,  ffmpeg::unknown_codec_id } },
    { video_format_id_t::argb32,    { ffmpeg::pixel_format_argb32,  ffmpeg::unknown_codec_id } },
    { video_format_id_t::bgra32,    { ffmpeg::pixel_format_bgra32,  ffmpeg::unknown_codec_id } },
    { video_format_id_t::rgba32,    { ffmpeg::pixel_format_rgba32,  ffmpeg::unknown_codec_id } },
    { video_format_id_t::gray8,     { ffmpeg::pixel_format_gray8,   ffmpeg::unknown_codec_id } },
    { video_format_id_t::gray16,    { ffmpeg::pixel_format_gray16,  ffmpeg::unknown_codec_id } },
    { video_format_id_t::gray16x,   { ffmpeg::pixel_format_gray16x, ffmpeg::unknown_codec_id } },
    { video_format_id_t::sbggr8,    { ffmpeg::pixel_format_sbggr8,  ffmpeg::unknown_codec_id } },
    { video_format_id_t::sgbrg8,    { ffmpeg::pixel_format_sgbrg8,  ffmpeg::unknown_codec_id } },
    { video_format_id_t::sgrbg8,    { ffmpeg::pixel_format_sgrbg8,  ffmpeg::unknown_codec_id } },
    { video_format_id_t::srggb8,    { ffmpeg::pixel_format_srggb8,  ffmpeg::unknown_codec_id } },
    { video_format_id_t::png,       { ffmpeg::unknown_pixel_format, ffmpeg::codec_id_png     } },
    { video_format_id_t::jpeg,      { ffmpeg::unknown_pixel_format, ffmpeg::codec_id_jpeg    } },
    { video_format_id_t::mjpeg,     { ffmpeg::unknown_pixel_format, ffmpeg::codec_id_mjpeg   } },
    { video_format_id_t::gif,       { ffmpeg::unknown_pixel_format, ffmpeg::codec_id_gif     } },
    { video_format_id_t::h265,      { ffmpeg::unknown_pixel_format, ffmpeg::codec_id_h265    } },
    { video_format_id_t::h264,      { ffmpeg::unknown_pixel_format, ffmpeg::codec_id_h264    } },
    { video_format_id_t::h263,      { ffmpeg::unknown_pixel_format, ffmpeg::codec_id_h263    } },
    { video_format_id_t::h263p,     { ffmpeg::unknown_pixel_format, ffmpeg::codec_id_h263p   } },
    { video_format_id_t::h261,      { ffmpeg::unknown_pixel_format, ffmpeg::codec_id_h261    } },
    { video_format_id_t::vp8,       { ffmpeg::unknown_pixel_format, ffmpeg::codec_id_vp8     } },
    { video_format_id_t::vp9,       { ffmpeg::unknown_pixel_format, ffmpeg::codec_id_vp9     } },
    { video_format_id_t::mpeg4,     { ffmpeg::unknown_pixel_format, ffmpeg::codec_id_mpeg4   } },
    { video_format_id_t::cpia,      { ffmpeg::unknown_pixel_format, ffmpeg::codec_id_cpia    } },
};


static const audio_table_t audio_format_table =
{
    { audio_format_id_t::undefined, { ffmpeg::unknown_sample_format,    ffmpeg::unknown_codec_id } },
    { audio_format_id_t::pcm8,      { ffmpeg::sample_format_pcm8,       ffmpeg::unknown_codec_id } },
    { audio_format_id_t::pcm16,     { ffmpeg::sample_format_pcm16,      ffmpeg::unknown_codec_id } },
    { audio_format_id_t::pcm32,     { ffmpeg::sample_format_pcm32,      ffmpeg::unknown_codec_id } },
    { audio_format_id_t::float32,   { ffmpeg::sample_format_float32,    ffmpeg::unknown_codec_id } },
    { audio_format_id_t::float64,   { ffmpeg::sample_format_float64,    ffmpeg::unknown_codec_id } },
    { audio_format_id_t::pcm8p,     { ffmpeg::sample_format_pcm8p,      ffmpeg::unknown_codec_id } },
    { audio_format_id_t::pcm16p,    { ffmpeg::sample_format_pcm16p,     ffmpeg::unknown_codec_id } },
    { audio_format_id_t::pcm32p,    { ffmpeg::sample_format_pcm32p,     ffmpeg::unknown_codec_id } },
    { audio_format_id_t::float32p,  { ffmpeg::sample_format_float32p,   ffmpeg::unknown_codec_id } },
    { audio_format_id_t::float64p,  { ffmpeg::sample_format_float64p,   ffmpeg::unknown_codec_id } },
    { audio_format_id_t::pcma,      { ffmpeg::unknown_sample_format,    ffmpeg::codec_id_pcma    } },
    { audio_format_id_t::pcmu,      { ffmpeg::unknown_sample_format,    ffmpeg::codec_id_pcmu    } },
    { audio_format_id_t::opus,      { ffmpeg::unknown_sample_format,    ffmpeg::codec_id_opus    } },
    { audio_format_id_t::aac,       { ffmpeg::unknown_sample_format,    ffmpeg::codec_id_aac     } },
};

template<typename F
         , typename InMap = std::unordered_map<F, ffmpeg::format_info_t>
         , typename OutMap = std::unordered_map<ffmpeg::format_id_t, F>>
OutMap create_format_map(const InMap& format_map)
{
    OutMap out;

    for (const auto& f : format_map)
    {
        if (f.second.format_id != ffmpeg::unknown_format_id)
        {
            out.emplace(f.second.format_id
                        , f.first);
        }
    }

    return out;
}

template<typename F
         , typename InMap = std::unordered_map<F, ffmpeg::format_info_t>
         , typename OutMap = std::unordered_map<ffmpeg::codec_id_t, F>>
OutMap create_codec_map(const InMap& format_map)
{
    OutMap out;

    for (const auto& f : format_map)
    {
        if (f.second.codec_id != ffmpeg::unknown_codec_id)
        {
            out.emplace(f.second.codec_id
                        , f.first);
        }
    }

    return out;
}

template<typename F>
const std::unordered_map<F, ffmpeg::format_info_t>& get_conversion_map();

template<>
const std::unordered_map<video_format_id_t,ffmpeg::format_info_t>& get_conversion_map()
{
    return video_format_table;
}

template<>
const std::unordered_map<audio_format_id_t,ffmpeg::format_info_t>& get_conversion_map()
{
    return audio_format_table;
}

static auto libav_video_format_table = create_format_map<video_format_id_t>(video_format_table);
static auto libav_video_codec_table = create_codec_map<video_format_id_t>(video_format_table);
static auto libav_audio_format_table = create_format_map<audio_format_id_t>(audio_format_table);
static auto libav_audio_codec_table = create_codec_map<audio_format_id_t>(audio_format_table);

template<typename F>
bool convert(const F& format_id, ffmpeg::format_info_t& format_info)
{
    const auto& conversion_table = detail::get_conversion_map<F>();
    if (auto it = conversion_table.find(format_id); it != conversion_table.end())
    {
        format_info = it->second;
        return true;
    }

    return false;
}

template<typename F>
bool convert(const ffmpeg::format_info_t& format_info, F& format_id)
{
    const auto& conversion_table = detail::get_conversion_map<F>();
    if (format_info.is_encoded())
    {
        static const auto codec_table = create_codec_map<F>(conversion_table);
        if (auto it = codec_table.find(format_info.codec_id); it != codec_table.end())
        {
            format_id = it->second;
            return true;
        }
    }
    else if (format_info.is_convertable())
    {
        static const auto format_table = create_format_map<F>(conversion_table);
        if (auto it = format_table.find(format_info.format_id); it != format_table.end())
        {
            format_id = it->second;
            return true;
        }
    }

    return false;
}

}

template<>
bool convert(const video_format_id_t& format_id, ffmpeg::format_info_t& format_info)
{
    return detail::convert<video_format_id_t>(format_id
                                              , format_info);
}

template<>
bool convert(const audio_format_id_t& format_id, ffmpeg::format_info_t& format_info)
{
    return detail::convert<audio_format_id_t>(format_id
                                              , format_info);
}

template<>
bool convert(const ffmpeg::format_info_t& format_info, video_format_id_t& format_id)
{
    return detail::convert(format_info
                           , format_id);
}

template<>
bool convert(const ffmpeg::format_info_t& format_info, audio_format_id_t& format_id)
{
    return detail::convert(format_info
                           , format_id);
}


}
