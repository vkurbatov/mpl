#include "libav_format_utils.h"

#include "utils/convert_utils.h"
#include "media_option_types.h"
#include "utils/option_helper.h"

#include "audio_format_impl.h"
#include "video_format_impl.h"
#include "format_utils.h"

#include "tools/ffmpeg/libav_base.h"

#include <unordered_map>
#include <cstdint>

namespace mpl::media::utils
{

namespace detail
{

using video_table_t = std::unordered_map<video_format_id_t
                                         , pt::ffmpeg::format_info_t>;

using audio_table_t = std::unordered_map<audio_format_id_t
                                         , pt::ffmpeg::format_info_t>;

static const video_table_t video_format_table =
{
    { video_format_id_t::undefined, { pt::ffmpeg::unknown_pixel_format, pt::ffmpeg::codec_id_none } },
    { video_format_id_t::yuv420p,   { pt::ffmpeg::pixel_format_yuv420p, pt::ffmpeg::codec_id_raw_video } },
    { video_format_id_t::yuv422p,   { pt::ffmpeg::pixel_format_yuv422p, pt::ffmpeg::codec_id_raw_video } },
    { video_format_id_t::yuv444p,   { pt::ffmpeg::pixel_format_yuv444p, pt::ffmpeg::codec_id_raw_video } },
    { video_format_id_t::yuv411p,   { pt::ffmpeg::pixel_format_yuv411p, pt::ffmpeg::codec_id_raw_video } },
    { video_format_id_t::yuyv,      { pt::ffmpeg::pixel_format_yuyv,    pt::ffmpeg::codec_id_raw_video } },
    { video_format_id_t::uyvy,      { pt::ffmpeg::pixel_format_uyvy,    pt::ffmpeg::codec_id_raw_video } },
    { video_format_id_t::yuv410,    { pt::ffmpeg::pixel_format_yuv410,  pt::ffmpeg::codec_id_raw_video } },
    { video_format_id_t::nv12,      { pt::ffmpeg::pixel_format_nv12,    pt::ffmpeg::codec_id_raw_video } },
    { video_format_id_t::nv21,      { pt::ffmpeg::pixel_format_nv21,    pt::ffmpeg::codec_id_raw_video } },
    { video_format_id_t::nv16,      { pt::ffmpeg::pixel_format_nv16,    pt::ffmpeg::codec_id_raw_video } },
    { video_format_id_t::bgr555,    { pt::ffmpeg::pixel_format_bgr555,  pt::ffmpeg::codec_id_raw_video } },
    { video_format_id_t::bgr555x,   { pt::ffmpeg::pixel_format_bgr555x, pt::ffmpeg::codec_id_raw_video } },
    { video_format_id_t::bgr565,    { pt::ffmpeg::pixel_format_bgr565,  pt::ffmpeg::codec_id_raw_video } },
    { video_format_id_t::bgr565x,   { pt::ffmpeg::pixel_format_bgr565x, pt::ffmpeg::codec_id_raw_video } },
    { video_format_id_t::rgb555,    { pt::ffmpeg::pixel_format_rgb555,  pt::ffmpeg::codec_id_raw_video } },
    { video_format_id_t::rgb555x,   { pt::ffmpeg::pixel_format_rgb555x, pt::ffmpeg::codec_id_raw_video } },
    { video_format_id_t::rgb565,    { pt::ffmpeg::pixel_format_rgb565,  pt::ffmpeg::codec_id_raw_video } },
    { video_format_id_t::rgb565x,   { pt::ffmpeg::pixel_format_rgb565x, pt::ffmpeg::codec_id_raw_video } },
    { video_format_id_t::bgr8,      { pt::ffmpeg::pixel_format_bgr8,    pt::ffmpeg::codec_id_raw_video } },
    { video_format_id_t::rgb8,      { pt::ffmpeg::pixel_format_rgb8,    pt::ffmpeg::codec_id_raw_video } },
    { video_format_id_t::bgr24,     { pt::ffmpeg::pixel_format_bgr24,   pt::ffmpeg::codec_id_raw_video } },
    { video_format_id_t::rgb24,     { pt::ffmpeg::pixel_format_rgb24,   pt::ffmpeg::codec_id_raw_video } },
    { video_format_id_t::bgr32,     { pt::ffmpeg::pixel_format_bgr32,   pt::ffmpeg::codec_id_raw_video } },
    { video_format_id_t::rgb32,     { pt::ffmpeg::pixel_format_rgb32,   pt::ffmpeg::codec_id_raw_video } },
    { video_format_id_t::abgr32,    { pt::ffmpeg::pixel_format_abgr32,  pt::ffmpeg::codec_id_raw_video } },
    { video_format_id_t::argb32,    { pt::ffmpeg::pixel_format_argb32,  pt::ffmpeg::codec_id_raw_video } },
    { video_format_id_t::bgra32,    { pt::ffmpeg::pixel_format_bgra32,  pt::ffmpeg::codec_id_raw_video } },
    { video_format_id_t::rgba32,    { pt::ffmpeg::pixel_format_rgba32,  pt::ffmpeg::codec_id_raw_video } },
    { video_format_id_t::gray8,     { pt::ffmpeg::pixel_format_gray8,   pt::ffmpeg::codec_id_raw_video } },
    { video_format_id_t::gray16,    { pt::ffmpeg::pixel_format_gray16,  pt::ffmpeg::codec_id_raw_video } },
    { video_format_id_t::gray16x,   { pt::ffmpeg::pixel_format_gray16x, pt::ffmpeg::codec_id_raw_video } },
    { video_format_id_t::sbggr8,    { pt::ffmpeg::pixel_format_sbggr8,  pt::ffmpeg::codec_id_raw_video } },
    { video_format_id_t::sgbrg8,    { pt::ffmpeg::pixel_format_sgbrg8,  pt::ffmpeg::codec_id_raw_video } },
    { video_format_id_t::sgrbg8,    { pt::ffmpeg::pixel_format_sgrbg8,  pt::ffmpeg::codec_id_raw_video } },
    { video_format_id_t::srggb8,    { pt::ffmpeg::pixel_format_srggb8,  pt::ffmpeg::codec_id_raw_video } },
    { video_format_id_t::png,       { pt::ffmpeg::unknown_pixel_format, pt::ffmpeg::codec_id_png     } },
    { video_format_id_t::jpeg,      { pt::ffmpeg::unknown_pixel_format, pt::ffmpeg::codec_id_jpeg    } },
    { video_format_id_t::mjpeg,     { pt::ffmpeg::unknown_pixel_format, pt::ffmpeg::codec_id_mjpeg   } },
    { video_format_id_t::gif,       { pt::ffmpeg::unknown_pixel_format, pt::ffmpeg::codec_id_gif     } },
    { video_format_id_t::h265,      { pt::ffmpeg::unknown_pixel_format, pt::ffmpeg::codec_id_h265    } },
    { video_format_id_t::h264,      { pt::ffmpeg::unknown_pixel_format, pt::ffmpeg::codec_id_h264    } },
    { video_format_id_t::h263,      { pt::ffmpeg::unknown_pixel_format, pt::ffmpeg::codec_id_h263    } },
    { video_format_id_t::h263p,     { pt::ffmpeg::unknown_pixel_format, pt::ffmpeg::codec_id_h263p   } },
    { video_format_id_t::h261,      { pt::ffmpeg::unknown_pixel_format, pt::ffmpeg::codec_id_h261    } },
    { video_format_id_t::vp8,       { pt::ffmpeg::unknown_pixel_format, pt::ffmpeg::codec_id_vp8     } },
    { video_format_id_t::vp9,       { pt::ffmpeg::unknown_pixel_format, pt::ffmpeg::codec_id_vp9     } },
    { video_format_id_t::mpeg4,     { pt::ffmpeg::unknown_pixel_format, pt::ffmpeg::codec_id_mpeg4   } },
    { video_format_id_t::cpia,      { pt::ffmpeg::unknown_pixel_format, pt::ffmpeg::codec_id_cpia    } },
};


static const audio_table_t audio_format_table =
{
    { audio_format_id_t::undefined, { pt::ffmpeg::unknown_sample_format,    pt::ffmpeg::codec_id_none } },
    { audio_format_id_t::pcm8,      { pt::ffmpeg::sample_format_pcm8,       pt::ffmpeg::codec_id_pcm8 } },
    { audio_format_id_t::pcm16,     { pt::ffmpeg::sample_format_pcm16,      pt::ffmpeg::codec_id_pcm16 } },
    { audio_format_id_t::pcm32,     { pt::ffmpeg::sample_format_pcm32,      pt::ffmpeg::codec_id_pcm32 } },
    { audio_format_id_t::float32,   { pt::ffmpeg::sample_format_float32,    pt::ffmpeg::codec_id_float32 } },
    { audio_format_id_t::float64,   { pt::ffmpeg::sample_format_float64,    pt::ffmpeg::codec_id_float64 } },
    { audio_format_id_t::pcm8p,     { pt::ffmpeg::sample_format_pcm8p,      pt::ffmpeg::codec_id_pcm8p } },
    { audio_format_id_t::pcm16p,    { pt::ffmpeg::sample_format_pcm16p,     pt::ffmpeg::codec_id_pcm16p } },
    { audio_format_id_t::pcm32p,    { pt::ffmpeg::sample_format_pcm32p,     pt::ffmpeg::codec_id_pcm32p } },
    { audio_format_id_t::float32p,  { pt::ffmpeg::sample_format_float32p,   pt::ffmpeg::codec_id_float32p } },
    { audio_format_id_t::float64p,  { pt::ffmpeg::sample_format_float64p,   pt::ffmpeg::codec_id_float64p } },
    { audio_format_id_t::pcma,      { pt::ffmpeg::unknown_sample_format,    pt::ffmpeg::codec_id_pcma    } },
    { audio_format_id_t::pcmu,      { pt::ffmpeg::unknown_sample_format,    pt::ffmpeg::codec_id_pcmu    } },
    { audio_format_id_t::opus,      { pt::ffmpeg::unknown_sample_format,    pt::ffmpeg::codec_id_opus    } },
    { audio_format_id_t::aac,       { pt::ffmpeg::unknown_sample_format,    pt::ffmpeg::codec_id_aac     } },
};

template<typename F
         , typename InMap = std::unordered_map<F, pt::ffmpeg::format_info_t>
         , typename OutMap = std::unordered_map<pt::ffmpeg::format_id_t, F>>
OutMap create_format_map(const InMap& format_map)
{
    OutMap out;

    for (const auto& f : format_map)
    {
        // if (out.find(f.second.format_id) == out.end())
        if (f.second.format_id != pt::ffmpeg::unknown_format_id)
        {
            out.emplace(f.second.format_id
                        , f.first);
        }
    }

    return out;
}

template<typename F
         , typename InMap = std::unordered_map<F, pt::ffmpeg::format_info_t>
         , typename OutMap = std::unordered_map<pt::ffmpeg::codec_id_t, F>>
OutMap create_codec_map(const InMap& format_map)
{
    OutMap out;

    for (const auto& f : format_map)
    {
        // if (out.find(f.second.codec_id) == out.end())
        if (f.second.codec_id > pt::ffmpeg::codec_id_none)
        {
            out.emplace(f.second.codec_id
                        , f.first);
        }
    }

    return out;
}

template<typename F>
const std::unordered_map<F, pt::ffmpeg::format_info_t>& get_conversion_map();

template<>
const std::unordered_map<video_format_id_t,pt::ffmpeg::format_info_t>& get_conversion_map()
{
    return video_format_table;
}

template<>
const std::unordered_map<audio_format_id_t,pt::ffmpeg::format_info_t>& get_conversion_map()
{
    return audio_format_table;
}

template<typename F>
bool convert(const F& format_id, pt::ffmpeg::format_info_t& format_info)
{
    if (format_id == F::undefined)
    {
        format_info = pt::ffmpeg::format_info_t::undefined();
        return true;
    }

    const auto& conversion_table = detail::get_conversion_map<F>();
    if (auto it = conversion_table.find(format_id); it != conversion_table.end())
    {
        format_info = it->second;
        return true;
    }

    return false;
}

template<typename F>
bool convert(const pt::ffmpeg::format_info_t& format_info, F& format_id)
{
    if (format_info.is_undefined())
    {
        format_id = F::undefined;
        return true;
    }

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

void convert_options(const i_option& option
                     , pt::ffmpeg::codec_info_t codec_info)
{

    option_reader reader(option);

    reader.get(opt_codec_name, codec_info.name);

    reader.get(opt_codec_bitrate, codec_info.codec_params.bitrate);
    reader.get(opt_codec_gop, codec_info.codec_params.gop);
    reader.get(opt_codec_frame_size, codec_info.codec_params.frame_size);
    reader.get(opt_codec_profile, codec_info.codec_params.profile);
    reader.get(opt_codec_level, codec_info.codec_params.level);

}

void convert_options(const i_option& option
                     , pt::ffmpeg::stream_info_t& stream_info)
{

    option_reader reader(option);

    reader.get(opt_fmt_track_id, stream_info.stream_id);
    reader.get(opt_codec_extra_data, stream_info.extra_data);
    convert_options(option
                    , stream_info.codec_info);
}

void convert_options(const pt::ffmpeg::codec_info_t& codec_info
                     , i_option& option)
{
    option_writer writer(option);

    if (codec_info.codec_params.bitrate != 0)
    {
        writer.set(opt_codec_bitrate, codec_info.codec_params.bitrate);
    }

    if (codec_info.codec_params.gop != 0)
    {
        writer.set(opt_codec_gop, codec_info.codec_params.gop);
    }

    if (codec_info.codec_params.frame_size != 0)
    {
        writer.set(opt_codec_frame_size, codec_info.codec_params.frame_size);
    }

    if (codec_info.codec_params.profile != 0)
    {
        writer.set(opt_codec_profile, codec_info.codec_params.profile);
    }

    if (codec_info.codec_params.level != 0)
    {
        writer.set(opt_codec_level, codec_info.codec_params.level);
    }
}

void convert_options(const pt::ffmpeg::stream_info_t& stream_info
                     , i_option& option)
{
    option_writer writer(option);

    if (stream_info.stream_id != pt::ffmpeg::no_stream)
    {
        writer.set(opt_fmt_track_id, stream_info.stream_id);
    }

    if (stream_info.extra_data)
    {
         writer.set(opt_codec_extra_data, stream_info.extra_data);
    }

}


} //detail

} //mpl::media::utils

namespace mpl::utils
{

template<>
bool convert(const media::video_format_id_t& format_id, pt::ffmpeg::format_info_t& format_info)
{
    return mpl::media::utils::detail::convert<media::video_format_id_t>(format_id
                                                                        , format_info);
}

template<>
bool convert(const media::audio_format_id_t& format_id, pt::ffmpeg::format_info_t& format_info)
{
    return mpl::media::utils::detail::convert<media::audio_format_id_t>(format_id
                                                                        , format_info);
}

template<>
bool convert(const pt::ffmpeg::format_info_t& format_info, media::video_format_id_t& format_id)
{
    return mpl::media::utils::detail::convert(format_info
                                                , format_id);
}

template<>
bool convert(const pt::ffmpeg::format_info_t& format_info, media::audio_format_id_t& format_id)
{
    return mpl::media::utils::detail::convert(format_info
                                                , format_id);
}

template<>
bool convert(const media::i_audio_format& format, pt::ffmpeg::stream_info_t& stream_info)
{
    pt::ffmpeg::format_info_t format_info;
    if (convert(format.format_id()
                , format_info))
    {
        stream_info.codec_info.id = format_info.codec_id;
        stream_info.media_info.media_type = pt::ffmpeg::media_type_t::audio;
        stream_info.media_info.audio_info.sample_format = format_info.format_id;
        stream_info.media_info.audio_info.sample_rate = format.sample_rate();
        stream_info.media_info.audio_info.channels = format.channels();
        mpl::media::utils::detail::convert_options(format.options()
                                                    , stream_info);

        return true;
    }

    return false;
}

template<>
bool convert(const media::i_video_format& format, pt::ffmpeg::stream_info_t& stream_info)
{
    pt::ffmpeg::format_info_t format_info;
    if (convert(format.format_id()
                , format_info))
    {
        stream_info.codec_info.id = format_info.codec_id;
        stream_info.media_info.media_type = pt::ffmpeg::media_type_t::video;
        stream_info.media_info.video_info.pixel_format = format_info.format_id;
        stream_info.media_info.video_info.size.width = format.width();
        stream_info.media_info.video_info.size.height = format.height();
        stream_info.media_info.video_info.fps = format.frame_rate();
        mpl::media::utils::detail::convert_options(format.options()
                                                    , stream_info);

        return true;
    }

    return false;
}

template<>
bool convert(const media::i_media_format& format, pt::ffmpeg::stream_info_t& stream_info)
{
    switch(format.media_type())
    {
        case media::media_type_t::audio:
        {
            return convert(static_cast<const media::i_audio_format&>(format)
                           , stream_info);
        }
        break;
        case media::media_type_t::video:
        {
            return convert(static_cast<const media::i_video_format&>(format)
                           , stream_info);
        }
        break;
        default:;
    }

    return false;
}

template<>
bool convert(const pt::ffmpeg::stream_info_t& stream_info
             , media::audio_format_impl& audio_format)
{
    if (stream_info.media_info.media_type == pt::ffmpeg::media_type_t::audio)
    {
        media::audio_format_id_t format_id = media::audio_format_id_t::undefined;
        if (convert(stream_info.format_info()
                    , format_id))
        {
            audio_format.set_format_id(format_id);
            audio_format.set_sample_rate(stream_info.media_info.audio_info.sample_rate);
            audio_format.set_channels(stream_info.media_info.audio_info.channels);

            mpl::media::utils::detail::convert_options(stream_info
                                                        , audio_format.options());

            return true;
        }
    }

    return false;
}

template<>
bool convert(const pt::ffmpeg::stream_info_t& stream_info
             , media::video_format_impl& video_format)
{
    if (stream_info.media_info.media_type == pt::ffmpeg::media_type_t::video)
    {
        media::video_format_id_t format_id = media::video_format_id_t::undefined;
        if (convert(stream_info.format_info()
                    , format_id))
        {
            video_format.set_format_id(format_id);
            video_format.set_width(stream_info.media_info.video_info.size.width);
            video_format.set_height(stream_info.media_info.video_info.size.height);
            video_format.set_frame_rate(stream_info.media_info.video_info.fps);

            mpl::media::utils::detail::convert_options(stream_info
                                                        , video_format.options());

            return true;
        }
    }

    return false;
}

}
