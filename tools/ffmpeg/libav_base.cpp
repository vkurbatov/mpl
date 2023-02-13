#include "libav_base.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libavutil/pixdesc.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
}

#include <sstream>
#include <chrono>
#include <thread>
#include <map>

namespace ffmpeg
{

static bool libav_register_flag = false;

const std::int32_t padding_size = AV_INPUT_BUFFER_PADDING_SIZE;

const codec_id_t codec_id_flv1 = static_cast<codec_id_t>(AV_CODEC_ID_FLV1);
const codec_id_t codec_id_h261 = static_cast<codec_id_t>(AV_CODEC_ID_H261);
const codec_id_t codec_id_h263 = static_cast<codec_id_t>(AV_CODEC_ID_H263);
const codec_id_t codec_id_h263p = static_cast<codec_id_t>(AV_CODEC_ID_H263P);
const codec_id_t codec_id_h264 = static_cast<codec_id_t>(AV_CODEC_ID_H264);
const codec_id_t codec_id_h265 = static_cast<codec_id_t>(AV_CODEC_ID_HEVC);
const codec_id_t codec_id_vp8 = static_cast<codec_id_t>(AV_CODEC_ID_VP8);
const codec_id_t codec_id_vp9 = static_cast<codec_id_t>(AV_CODEC_ID_VP9);
const codec_id_t codec_id_mpeg4 = static_cast<codec_id_t>(AV_CODEC_ID_MPEG4);
const codec_id_t codec_id_cpia = static_cast<codec_id_t>(AV_CODEC_ID_CPIA);
const codec_id_t codec_id_mjpeg = static_cast<codec_id_t>(AV_CODEC_ID_MJPEG);
const codec_id_t codec_id_jpeg = static_cast<codec_id_t>(AV_CODEC_ID_JPEG2000);
const codec_id_t codec_id_gif = static_cast<codec_id_t>(AV_CODEC_ID_GIF);
const codec_id_t codec_id_png = static_cast<codec_id_t>(AV_CODEC_ID_PNG);
const codec_id_t codec_id_raw_video = static_cast<codec_id_t>(AV_CODEC_ID_RAWVIDEO);

const codec_id_t codec_id_aac = static_cast<codec_id_t>(AV_CODEC_ID_AAC);
const codec_id_t codec_id_opus = static_cast<codec_id_t>(AV_CODEC_ID_OPUS);
const codec_id_t codec_id_pcma = static_cast<codec_id_t>(AV_CODEC_ID_PCM_ALAW);
const codec_id_t codec_id_pcmu = static_cast<codec_id_t>(AV_CODEC_ID_PCM_MULAW);

const codec_id_t codec_id_first_audio = static_cast<codec_id_t>(AV_CODEC_ID_FIRST_AUDIO);

const codec_id_t codec_id_none = static_cast<codec_id_t>(AV_CODEC_ID_NONE);

//extern const pixel_format_t default_pixel_format = static_cast<pixel_format_t>(AV_PIX_FMT_YUV420P);
const pixel_format_t default_pixel_format = static_cast<pixel_format_t>(AV_PIX_FMT_YUV420P);
const sample_format_t default_sample_format = static_cast<sample_format_t>(AV_SAMPLE_FMT_S16);

const pixel_format_t pixel_format_none = static_cast<pixel_format_t>(AV_PIX_FMT_NONE);

const pixel_format_t pixel_format_bgr8 = static_cast<pixel_format_t>(AV_PIX_FMT_BGR8);
const pixel_format_t pixel_format_rgb8 = static_cast<pixel_format_t>(AV_PIX_FMT_RGB8);
const pixel_format_t pixel_format_bgr15 = static_cast<pixel_format_t>(AV_PIX_FMT_BGR555);
const pixel_format_t pixel_format_rgb15 = static_cast<pixel_format_t>(AV_PIX_FMT_RGB555);
const pixel_format_t pixel_format_bgr16 = static_cast<pixel_format_t>(AV_PIX_FMT_BGR565);
const pixel_format_t pixel_format_rgb16 = static_cast<pixel_format_t>(AV_PIX_FMT_RGB565);
const pixel_format_t pixel_format_bgr24 = static_cast<pixel_format_t>(AV_PIX_FMT_BGR24);
const pixel_format_t pixel_format_rgb24 = static_cast<pixel_format_t>(AV_PIX_FMT_RGB24);
const pixel_format_t pixel_format_bgr32 = static_cast<pixel_format_t>(AV_PIX_FMT_BGR32);
const pixel_format_t pixel_format_rgb32 = static_cast<pixel_format_t>(AV_PIX_FMT_RGB32);
const pixel_format_t pixel_format_bgra = static_cast<pixel_format_t>(AV_PIX_FMT_BGRA);
const pixel_format_t pixel_format_rgba = static_cast<pixel_format_t>(AV_PIX_FMT_RGBA);
const pixel_format_t pixel_format_abgr = static_cast<pixel_format_t>(AV_PIX_FMT_ABGR);
const pixel_format_t pixel_format_argb = static_cast<pixel_format_t>(AV_PIX_FMT_ARGB);
const pixel_format_t pixel_format_gray8 = static_cast<pixel_format_t>(AV_PIX_FMT_GRAY8);
const pixel_format_t pixel_format_nv12 = static_cast<pixel_format_t>(AV_PIX_FMT_NV12);
const pixel_format_t pixel_format_nv21 = static_cast<pixel_format_t>(AV_PIX_FMT_NV21);
const pixel_format_t pixel_format_yuv420p = static_cast<pixel_format_t>(AV_PIX_FMT_YUV420P);
const pixel_format_t pixel_format_yuv422p = static_cast<pixel_format_t>(AV_PIX_FMT_YUV422P);

const pixel_format_t pixel_format_yuv444p = static_cast<pixel_format_t>(AV_PIX_FMT_YUV444P);
const pixel_format_t pixel_format_yuv411p = static_cast<pixel_format_t>(AV_PIX_FMT_YUV411P);
const pixel_format_t pixel_format_yuyv = static_cast<pixel_format_t>(AV_PIX_FMT_YUYV422);
const pixel_format_t pixel_format_uyvy = static_cast<pixel_format_t>(AV_PIX_FMT_UYVY422);
const pixel_format_t pixel_format_yuv410 = static_cast<pixel_format_t>(AV_PIX_FMT_YUV410P);

const pixel_format_t pixel_format_nv16 = static_cast<pixel_format_t>(AV_PIX_FMT_NV16);
const pixel_format_t pixel_format_bgr555 = static_cast<pixel_format_t>(AV_PIX_FMT_BGR555LE);
const pixel_format_t pixel_format_bgr555x = static_cast<pixel_format_t>(AV_PIX_FMT_BGR555BE);
const pixel_format_t pixel_format_bgr565 = static_cast<pixel_format_t>(AV_PIX_FMT_BGR565LE);
const pixel_format_t pixel_format_bgr565x = static_cast<pixel_format_t>(AV_PIX_FMT_BGR565BE);
const pixel_format_t pixel_format_rgb555 = static_cast<pixel_format_t>(AV_PIX_FMT_RGB555LE);
const pixel_format_t pixel_format_rgb555x = static_cast<pixel_format_t>(AV_PIX_FMT_RGB555BE);
const pixel_format_t pixel_format_rgb565 = static_cast<pixel_format_t>(AV_PIX_FMT_RGB565LE);
const pixel_format_t pixel_format_rgb565x = static_cast<pixel_format_t>(AV_PIX_FMT_RGB565BE);

const pixel_format_t pixel_format_abgr32 = static_cast<pixel_format_t>(AV_PIX_FMT_ABGR);
const pixel_format_t pixel_format_argb32 = static_cast<pixel_format_t>(AV_PIX_FMT_ARGB);
const pixel_format_t pixel_format_bgra32 = static_cast<pixel_format_t>(AV_PIX_FMT_BGRA);
const pixel_format_t pixel_format_rgba32 = static_cast<pixel_format_t>(AV_PIX_FMT_RGBA);

const pixel_format_t pixel_format_gray16 = static_cast<pixel_format_t>(AV_PIX_FMT_GRAY16LE);
const pixel_format_t pixel_format_gray16x = static_cast<pixel_format_t>(AV_PIX_FMT_GRAY16BE);
const pixel_format_t pixel_format_sbggr8 = static_cast<pixel_format_t>(AV_PIX_FMT_BAYER_BGGR8);
const pixel_format_t pixel_format_sgbrg8 = static_cast<pixel_format_t>(AV_PIX_FMT_BAYER_GBRG8);
const pixel_format_t pixel_format_sgrbg8 = static_cast<pixel_format_t>(AV_PIX_FMT_BAYER_GRBG8);
const pixel_format_t pixel_format_srggb8 = static_cast<pixel_format_t>(AV_PIX_FMT_BAYER_RGGB8);

const sample_format_t sample_format_none = static_cast<sample_format_t>(AV_SAMPLE_FMT_NONE);
const sample_format_t sample_format_pcm8 = static_cast<sample_format_t>(AV_SAMPLE_FMT_U8);
const sample_format_t sample_format_pcm16 = static_cast<sample_format_t>(AV_SAMPLE_FMT_S16);
const sample_format_t sample_format_pcm32 = static_cast<sample_format_t>(AV_SAMPLE_FMT_S32);
const sample_format_t sample_format_float32 = static_cast<sample_format_t>(AV_SAMPLE_FMT_FLT);
const sample_format_t sample_format_float64 = static_cast<sample_format_t>(AV_SAMPLE_FMT_DBL);
const sample_format_t sample_format_pcm8p = static_cast<sample_format_t>(AV_SAMPLE_FMT_U8P);
const sample_format_t sample_format_pcm16p = static_cast<sample_format_t>(AV_SAMPLE_FMT_S16P);
const sample_format_t sample_format_pcm32p = static_cast<sample_format_t>(AV_SAMPLE_FMT_S32P);
const sample_format_t sample_format_float32p = static_cast<sample_format_t>(AV_SAMPLE_FMT_FLTP);
const sample_format_t sample_format_float64p = static_cast<sample_format_t>(AV_SAMPLE_FMT_DBLP);

const stream_parse_type_t stream_parse_none = static_cast<stream_parse_type_t>(AVSTREAM_PARSE_NONE);
const stream_parse_type_t stream_parse_full = static_cast<stream_parse_type_t>(AVSTREAM_PARSE_FULL);
const stream_parse_type_t stream_parse_headers = static_cast<stream_parse_type_t>(AVSTREAM_PARSE_HEADERS);
const stream_parse_type_t stream_parse_timestamp = static_cast<stream_parse_type_t>(AVSTREAM_PARSE_TIMESTAMPS);
const stream_parse_type_t stream_parse_full_once = static_cast<stream_parse_type_t>(AVSTREAM_PARSE_FULL_ONCE);
const stream_parse_type_t stream_parse_full_raw = static_cast<stream_parse_type_t>(AVSTREAM_PARSE_FULL_RAW);

const std::string libav_param_name_thread_count     = "libav_threads";
const std::string libav_param_name_bitrate          = "libav_bitrate";
const std::string libav_param_name_gop              = "libav_gop";
const std::string libav_param_name_frame_size       = "libav_frame_size";
const std::string libav_param_name_global_header    = "libav_global_header";
const std::string libav_param_name_profile          = "libav_profile";
const std::string libav_param_name_level            = "libav_level";
const std::string libav_param_name_qmin             = "libav_qmin";
const std::string libav_param_name_qmax             = "libav_qmax";

typedef std::map<std::string, custom_parameter_t> custom_parameter_dictionary_t;

const custom_parameter_dictionary_t custom_parameter_dictionary =
{
    { libav_param_name_thread_count     , custom_parameter_t::thread_count  },
    { libav_param_name_bitrate          , custom_parameter_t::bitrate       },
    { libav_param_name_gop              , custom_parameter_t::gop           },
    { libav_param_name_frame_size       , custom_parameter_t::frame_size    },
    { libav_param_name_global_header    , custom_parameter_t::global_header },
    { libav_param_name_profile          , custom_parameter_t::profile       },
    { libav_param_name_level            , custom_parameter_t::level         },
    { libav_param_name_qmin             , custom_parameter_t::qmin          },
    { libav_param_name_qmax             , custom_parameter_t::qmax          }
};

std::string error_to_string(int32_t av_error)
{
    char err[AV_ERROR_MAX_STRING_SIZE] = {};
    av_strerror(av_error, err, AV_ERROR_MAX_STRING_SIZE);
    return err;               
}

custom_parameter_t check_custom_param(const std::string param_name)
{
    const auto it = custom_parameter_dictionary.find(param_name);

    return it == custom_parameter_dictionary.end()
            ? custom_parameter_t::unknown
            : it->second;
}

device_class_list_t device_info_t::device_class_list(media_type_t media_type
                                                     , bool is_source)
{
    device_class_list_t device_class_list;

    if (media_type != media_type_t::data)
    {
        if (is_source)
        {
            auto get_device = media_type == media_type_t::audio
                    ? av_input_audio_device_next
                    : av_input_video_device_next;

            for (AVInputFormat* input_format = get_device(nullptr)
                    ; input_format != nullptr
                    ; input_format = get_device(input_format))
            {
                if (input_format->name != nullptr)
                {
                    device_class_list.emplace_back(input_format->name);
                    if (device_class_list.back().find("video4linux") != std::string::npos)
                    {
                        device_class_list.back() = "v4l2";
                    }
                }
            }
        }
        else
        {
            auto get_device = media_type == media_type_t::audio
                    ? av_output_audio_device_next
                    : av_output_video_device_next;

            for (AVOutputFormat* output_format = get_device(nullptr)
                    ; output_format != nullptr
                    ; output_format = get_device(output_format))
            {
                if (output_format->name != nullptr)
                {
                    device_class_list.emplace_back(output_format->name);
                }
            }
        }
    }

    return device_class_list;
}

device_info_t::list_t device_info_t::device_list(media_type_t media_type
                                              , bool is_source
                                              , const std::string& device_class)
{
    list_t device_info_list;

    auto class_list = device_class_list(media_type
                                        , is_source);

    if (!class_list.empty())
    {
        for (const auto& c : class_list)
        {
            if (device_class.empty()
                    || c == device_class)
            {
                AVDeviceInfoList* av_device_list = nullptr;

                std::int32_t result = is_source
                                      ? avdevice_list_input_sources(nullptr
                                                                    , c.c_str()
                                                                    , nullptr
                                                                    , &av_device_list)
                                      : avdevice_list_output_sinks(nullptr
                                                                   , c.c_str()
                                                                   , nullptr
                                                                   , &av_device_list);

                if (result >= 0)
                {
                    for (auto i = 0; i < av_device_list->nb_devices; i++)
                    {
                        auto device = av_device_list->devices[i];
                        device_info_t device_info(media_type
                                                  , device->device_name
                                                  , device->device_description
                                                  , c
                                                  , is_source);
                        device_info_list.emplace_back(std::move(device_info));
                    }
                }

                if (av_device_list != nullptr)
                {
                    av_freep(av_device_list);
                }
            }
        }
    }

    return device_info_list;
}

device_info_t::device_info_t(media_type_t media_type
                             , const std::string &name
                             , const std::string &description
                             , const std::string &device_class
                             , bool is_source)
    : media_type(media_type)
    , name(name)
    , description(description)
    , device_class(device_class)
    , is_source(is_source)
{

}

std::string device_info_t::to_uri() const
{
    std::string device_uri = device_class;
    device_uri.append("://");
    device_uri.append(name);
    return device_uri;
}

uint32_t audio_info_t::bps(sample_format_t sample_format)
{
    return av_get_bytes_per_sample(static_cast<AVSampleFormat>(sample_format)) * 8;
}

std::size_t audio_info_t::sample_size(sample_format_t sample_format, uint32_t channels)
{
   return (bps(sample_format) * channels) / 8;
}

std::string audio_info_t::format_name(sample_format_t sample_format)
{
    return av_get_sample_fmt_name(static_cast<AVSampleFormat>(sample_format));
}

bool audio_info_t::is_planar(sample_format_t sample_format)
{
    return av_sample_fmt_is_planar(static_cast<AVSampleFormat>(sample_format));
}

audio_info_t::audio_info_t(uint32_t sample_rate
                           , uint32_t channels
                           , sample_format_t sample_format)
    : sample_rate(sample_rate)
    , channels(channels)
    , sample_format(sample_format)
{

}

bool audio_info_t::operator ==(const audio_info_t &audio_info) const
{
    return sample_rate == audio_info.sample_rate
            && channels == audio_info.channels
            && sample_format == audio_info.sample_format;
}

bool audio_info_t::operator !=(const audio_info_t &audio_info) const
{
    return ! operator ==(audio_info);
}

uint32_t audio_info_t::bps() const
{
    return bps(sample_format);
}

bool audio_info_t::is_planar() const
{
    return is_planar(sample_format);
}

std::size_t audio_info_t::sample_size() const
{
    return sample_size(sample_format, channels);
}

std::string audio_info_t::format_name() const
{
    return format_name(sample_format);
}

uint32_t video_info_t::bpp(pixel_format_t pixel_format)
{
    return av_get_bits_per_pixel(av_pix_fmt_desc_get(static_cast<AVPixelFormat>(pixel_format)));
}

std::size_t video_info_t::frame_size(pixel_format_t pixel_format
                                     , const frame_size_t &size
                                     , std::int32_t align)
{
    return av_image_get_buffer_size(static_cast<AVPixelFormat>(pixel_format)
                                    , size.width
                                    , size.height
                                    , align);
}

std::string video_info_t::format_name(pixel_format_t pixel_format)
{
    return av_get_pix_fmt_name(static_cast<AVPixelFormat>(pixel_format));
}

std::size_t video_info_t::planes(pixel_format_t pixel_format)
{
    return av_pix_fmt_count_planes(static_cast<AVPixelFormat>(pixel_format));
}

std::size_t video_info_t::plane_width(pixel_format_t pixel_format
                                      , uint32_t width
                                      , uint32_t plane_idx)
{
    return av_image_get_linesize(static_cast<AVPixelFormat>(pixel_format)
                                 , width
                                 , plane_idx);
}

plane_sizes_t video_info_t::plane_sizes(pixel_format_t pixel_format
                                        , const frame_size_t &size
                                        , std::int32_t align)
{
    plane_sizes_t plane_sizes;

    std::uint8_t* slices[max_planes] = {};
    std::int32_t strides[max_planes] = {};

    auto frame_size = av_image_fill_arrays(slices
                                            , strides
                                            , nullptr
                                            , static_cast<AVPixelFormat>(pixel_format)
                                            , size.width
                                            , size.height
                                            , align);

    if (frame_size > 0)
    {
        for (int i = 0; i < static_cast<std::int32_t>(max_planes) && strides[i] > 0; i++)
        {
            std::int32_t sz = strides[i + 1] == 0 || max_planes == i + 1
                    ? frame_size - (slices[i] - slices[0])
                    : slices[i + 1] - slices[i];

            plane_sizes.push_back( { strides[i], sz / strides[i] } );
        }
    }

    return plane_sizes;
}

std::size_t video_info_t::split_slices(pixel_format_t pixel_format
                                        , const frame_size_t &size
                                        , void *slices[]
                                        , const void *data
                                        , int32_t align)
{
    std::size_t result = 0;
    std::size_t offset = 0;

    for (const auto& sz : plane_sizes(pixel_format
                                      , size
                                      , align))
    {
        slices[result] = const_cast<std::uint8_t*>(static_cast<const std::uint8_t*>(data) + offset);
        offset += sz.size();
        result++;
    }

    return result;
}

plane_list_t video_info_t::split_planes(pixel_format_t pixel_format
                                        , const frame_size_t &size
                                        , const void *data
                                        , int32_t align)
{
    plane_list_t plane_list;

    std::uint8_t* slices[max_planes] = {};
    std::int32_t strides[max_planes] = {};

    auto frame_size = av_image_fill_arrays(slices
                                    , strides
                                    , static_cast<const std::uint8_t*>(data)
                                    , static_cast<AVPixelFormat>(pixel_format)
                                    , size.width
                                    , size.height
                                    , align);

    if (frame_size > 0)
    {
        for (int i = 0; i < static_cast<std::int32_t>(max_planes) && strides[i] > 0; i++)
        {
            std::int32_t sz = strides[i + 1] == 0 || max_planes == i + 1
                    ? frame_size - (slices[i] - slices[0])
                    : slices[i + 1] - slices[i];

            plane_list.push_back({ static_cast<void*>(slices[i])
                                  , { strides[i], sz / strides[i] }
                                 });
        }
    }

    return plane_list;
}

bool video_info_t::blackout(pixel_format_t pixel_format
                            , const frame_size_t &size
                            , void *slices[])
{
    std::int32_t    linesizes[max_planes] = {};

    if (av_image_fill_linesizes(linesizes
                                , static_cast<AVPixelFormat>(pixel_format)
                                , size.width) >= 0)
    {
        ptrdiff_t lines[] = { linesizes[0]
                              , linesizes[1]
                              , linesizes[2]
                              , linesizes[3] };
        return av_image_fill_black(*(reinterpret_cast<uint8_t***>(&slices))
                            , lines
                            , static_cast<AVPixelFormat>(pixel_format)
                            , AVCOL_RANGE_MPEG
                            , size.width
                            , size.height) >= 0;
    }

    return false;
}

bool video_info_t::blackout(pixel_format_t pixel_format
                            , const frame_size_t &size
                            , void *data
                            , std::int32_t align)
{
    std::uint8_t* slices[max_planes] = {};
    std::int32_t strides[max_planes] = {};

    auto frame_size = av_image_fill_arrays(slices
                                        , strides
                                        , static_cast<const std::uint8_t*>(data)
                                        , static_cast<AVPixelFormat>(pixel_format)
                                        , size.width
                                        , size.height
                                        , align);

    if (frame_size > 0)
    {
        ptrdiff_t lines[] = { strides[0]
                              , strides[1]
                              , strides[2]
                              , strides[3] };
        return av_image_fill_black(slices
                            , lines
                            , static_cast<AVPixelFormat>(pixel_format)
                            , AVCOL_RANGE_MPEG
                            , size.width
                            , size.height) >= 0;
        return true;
    }

    return false;
}

video_info_t::video_info_t(int32_t width
                           , int32_t height
                           , uint32_t fps
                           , pixel_format_t pixel_format)
    : video_info_t({ width, height}
                   , fps
                   , pixel_format)
{

}

video_info_t::video_info_t(frame_size_t size
                           , uint32_t fps
                           , pixel_format_t pixel_format)
    : size(size)
    , fps(fps)
    , pixel_format(pixel_format)
{

}

bool video_info_t::operator ==(const video_info_t &video_info) const
{
    return fps == video_info.fps
            && size == video_info.size
            && pixel_format == video_info.pixel_format;
}

bool video_info_t::operator !=(const video_info_t &video_info) const
{
    return ! operator ==(video_info);
}

uint32_t video_info_t::bpp() const
{
    return bpp(pixel_format);
}

std::size_t video_info_t::frame_size(std::int32_t align) const
{
    return frame_size(pixel_format
                      , size
                      , align);
}

std::string video_info_t::format_name() const
{
    return format_name(pixel_format);
}

std::size_t video_info_t::planes() const
{
    return planes(pixel_format);
}

std::size_t video_info_t::plane_width(uint32_t plane_idx) const
{
    return plane_width(pixel_format
                       , size.width
                       , plane_idx);
}

plane_sizes_t video_info_t::plane_sizes() const
{
    return plane_sizes(pixel_format
                       , size);
}

std::size_t video_info_t::split_slices(void *slices[]
                                       , const void *data
                                       , int32_t align) const
{
    return split_slices(pixel_format
                        , size
                        , slices
                        , data
                        , align);
}

plane_list_t video_info_t::split_planes(const void *data
                                        , int32_t align)
{
    return split_planes(pixel_format
                        , size
                        , data
                        , align);
}

bool video_info_t::blackout(void *slices[]) const
{
    return blackout(pixel_format
                    , size
                    , slices);
}

bool video_info_t::blackout(void *data
                            , int32_t align) const
{
    return blackout(pixel_format
                    , size
                    , data
                    , align);
}


media_info_t::media_info_t(const audio_info_t &audio_info)
    : media_type(media_type_t::audio)
    , audio_info(audio_info)
    , video_info()
{

}

media_info_t::media_info_t(const video_info_t &video_info)
    : media_type(media_type_t::video)
    , video_info(video_info)
{

}

bool media_info_t::operator==(const media_info_t &media_info) const
{
    if (media_type == media_info.media_type)
    {
        switch(media_type)
        {
            case media_type_t::video:
                 return video_info == media_info.video_info;
            break;
            case media_type_t::audio:
                return audio_info == media_info.audio_info;
            break;
            case media_type_t::data:
                return true;
            break;
        }
    }

    return false;
}

bool media_info_t::operator!=(const media_info_t &media_info) const
{
    return !operator == (media_info);
}

uint32_t media_info_t::sample_rate() const
{
    switch(media_type)
    {
        case media_type_t::audio:
            return audio_info.sample_rate;
        break;
        case media_type_t::video:
            return video_sample_rate;
        break;
    }

    return 0;
}

std::string media_info_t::to_string() const
{
    std::stringstream ss;

    switch(media_type)
    {
        case media_type_t::audio:
            ss << "A[" << audio_info.sample_rate
               << "/" << audio_info.bps()
               << "/" << audio_info.channels
               << "]";
        break;
        case media_type_t::video:
            ss << "V[" << video_info.size.width
               << "x" << video_info.size.height
               << "@" << video_info.fps
               << "]";
        break;
        default:
            ss << "D";
        break;
    }

    return ss.str();
}

frame_info_t::frame_info_t(const media_info_t &media_info
                           , int64_t pts
                           , int64_t dts
                           , int32_t id
                           , codec_id_t codec_id
                           , bool key_frame)
    : media_info(media_info)
    , pts(pts)
    , dts(dts)
    , id(id)
    , codec_id(codec_id)
    , key_frame(key_frame)
{

}

bool frame_info_t::is_encoded() const
{
    return codec_id > 0
            && codec_id != codec_id_raw_video;
}

std::string frame_info_t::to_string() const
{
    return media_info.to_string();
}

int64_t frame_info_t::timestamp() const
{
    return dts != AV_NOPTS_VALUE
            ? dts
            : pts != AV_NOPTS_VALUE
                ? pts
                : 0;
}

bool frame_info_t::is_timestamp() const
{
    return dts != AV_NOPTS_VALUE
            || pts != AV_NOPTS_VALUE;
}

format_info_t::format_info_t(format_id_t format_id
                             , codec_id_t codec_id)
    : format_id(format_id)
    , codec_id(codec_id)
{

}

bool format_info_t::is_valid() const
{
    return format_id > unknown_format_id
            || codec_id > codec_id_none;
}

bool format_info_t::is_encoded() const
{
    return codec_id > codec_id_none;
}

bool format_info_t::is_convertable() const
{
    return !is_encoded()
            && format_id != unknown_format_id;
}

extra_data_t stream_info_t::create_extra_data(const void *extra_data
                                              , std::size_t extra_data_size
                                              , bool need_padding)
{
    if (extra_data != nullptr
        && extra_data_size > 0)
    {
        extra_data_t extra_buffer = std::make_shared<media_data_t>(extra_data_size + (need_padding
                                                                    ? AV_INPUT_BUFFER_PADDING_SIZE
                                                                    : 0)
                                                                    , 0);

        memcpy(extra_buffer->data()
               , extra_data
               , extra_data_size);

        return extra_buffer;
    }

    return nullptr;
}

stream_info_t::stream_info_t(stream_id_t stream_id
                             , const codec_info_t &codec_info
                             , const media_info_t &media_info
                             , const void *extra_data
                             , std::size_t extra_data_size
                             , bool need_extra_padding)
    : stream_id(stream_id)
    , codec_info(codec_info)
    , media_info(media_info)
    , extra_data(std::move(create_extra_data(extra_data
                                             , extra_data_size
                                             , need_extra_padding)))
{

}

format_info_t stream_info_t::format_info() const
{
    switch(media_info.media_type)
    {
        case media_type_t::audio:
            return { media_info.audio_info.sample_format, codec_info.id };
        break;
        case media_type_t::video:
            return { media_info.video_info.pixel_format, codec_info.id };
        break;
        default:;
    }

    return {};
}

std::string stream_info_t::to_string() const
{

    std::stringstream ss;

    ss << "stream #" << stream_id << ":" << media_info.to_string()
       << ":c=" << codec_info.name;

    return ss.str();

}

fragment_info_t::fragment_info_t(int32_t x
                                 , int32_t y
                                 , int32_t width
                                 , int32_t height
                                 , int32_t frame_width
                                 , int32_t frame_height
                                 , pixel_format_t pixel_format)
    : fragment_info_t({ x, y, width, height }
                      , { frame_width, frame_height }
                      , pixel_format)
{

}

fragment_info_t::fragment_info_t(const frame_rect_t& frame_rect
                                 , const frame_size_t& frame_size
                                 , pixel_format_t pixel_format)
    : frame_rect(frame_rect)
    , frame_size(frame_size)
    , pixel_format(pixel_format)
{

}

fragment_info_t::fragment_info_t(const frame_size_t &frame_size
                                 , pixel_format_t pixel_format)
    : fragment_info_t({ 0, 0, frame_size.width, frame_size.height }
                      , { frame_size.width, frame_size.height }
                      , pixel_format)
{

}

size_t fragment_info_t::get_fragment_size(std::int32_t align) const
{
    return video_info_t::frame_size(pixel_format, frame_rect.size, align);
}


size_t fragment_info_t::get_frame_size(std::int32_t align) const
{
    return video_info_t::frame_size(pixel_format, frame_size, align);
}

void fragment_info_t::adjust_align(const frame_size_t &align)
{
    frame_rect.offset.x -= frame_rect.offset.x % align.width;
    frame_rect.offset.y -= frame_rect.offset.y % align.height;
    frame_rect.size.width -= frame_rect.size.width % align.width;
    frame_rect.size.height -= frame_rect.size.height % align.height;
    frame_size.width -= frame_size.width % align.width;
    frame_size.height -= frame_size.height % align.height;
}

bool fragment_info_t::is_full() const
{
    return frame_rect.offset.is_null()
            && frame_rect.size == frame_size;
}

bool fragment_info_t::is_convertable() const
{
    return pixel_format != pixel_format_none
            && frame_rect.is_join(frame_size);
}

bool fragment_info_t::operator ==(const fragment_info_t &fragment_info) const
{
    return frame_rect == fragment_info.frame_rect
            && frame_size == fragment_info.frame_size
            && pixel_format == fragment_info.pixel_format;
}

bool fragment_info_t::operator !=(const fragment_info_t &fragment_info) const
{
    return !operator ==(fragment_info);
}

std::string codec_info_t::codec_name(codec_id_t id)
{    
    return avcodec_get_name(static_cast<AVCodecID>(id));
}

pixel_formats_t codec_info_t::supported_video_formats(codec_id_t id)
{
    pixel_formats_t pixel_formats;
    auto codec = avcodec_find_encoder(static_cast<AVCodecID>(id));
    if (codec != nullptr && codec->pix_fmts != nullptr)
    {
        auto i = 0;
        while (codec->pix_fmts[i] != AV_PIX_FMT_NONE)
        {
            pixel_formats.push_back(codec->pix_fmts[i]);
            i++;
        }
    }
    return pixel_formats;
}

sample_formats_t codec_info_t::supported_audio_formats(codec_id_t id)
{
    sample_formats_t sample_formats;
    auto codec = avcodec_find_encoder(static_cast<AVCodecID>(id));
    if (codec != nullptr && codec->sample_fmts != nullptr)
    {
        auto i = 0;
        while (codec->sample_fmts[i] != AV_SAMPLE_FMT_NONE)
        {
            sample_formats.push_back(codec->sample_fmts[i]);
            i++;
        }
    }
    return sample_formats;
}

pixel_formats_t codec_info_t::supported_video_formats(const std::string &name)
{
    pixel_formats_t pixel_formats;
    auto codec = avcodec_find_encoder_by_name(name.c_str());
    if (codec != nullptr && codec->pix_fmts != nullptr)
    {
        auto i = 0;
        while (codec->pix_fmts[i] != AV_PIX_FMT_NONE)
        {
            pixel_formats.push_back(codec->pix_fmts[i]);
            i++;
        }
    }
    return pixel_formats;
}

sample_formats_t codec_info_t::supported_audio_formats(const std::string &name)
{
    sample_formats_t sample_formats;
    auto codec = avcodec_find_encoder_by_name(name.c_str());
    if (codec != nullptr && codec->sample_fmts != nullptr)
    {
        auto i = 0;
        while (codec->sample_fmts[i] != AV_SAMPLE_FMT_NONE)
        {
            sample_formats.push_back(codec->sample_fmts[i]);
            i++;
        }
    }
    return sample_formats;
}

codec_info_t::codec_info_t(codec_id_t id
                           , const std::string &name
                           , const codec_params_t codec_params)
    : id(id)
    , name(name)
    , codec_params(codec_params)
{
    if (this->name.empty() && id != codec_id_none)
    {
        this->name = codec_name(id);
    }
}

bool codec_info_t::is_coded() const
{
    return id > codec_id_none
            && id != codec_id_raw_video;
}

std::string codec_info_t::to_string() const
{
    return name.empty()
            ? codec_name(id)
            : name;
}

pixel_formats_t codec_info_t::supported_video_formats() const
{
    return name.empty()
            ? supported_video_formats(id)
            : supported_video_formats(name);
}

sample_formats_t codec_info_t::supported_audio_formats() const
{
    return name.empty()
            ? supported_audio_formats(id)
            : supported_audio_formats(name);
}

codec_params_t::codec_params_t(std::int32_t bitrate
                               , std::int32_t gop
                               , std::int32_t frame_size
                               , std::uint32_t flags1
                               , std::uint32_t flags2
                               , std::int32_t profile
                               , std::int32_t level
                               , stream_parse_type_t parse_type)
    : bitrate(bitrate)
    , gop(gop)
    , frame_size(frame_size)
    , flags1(flags1)
    , flags2(flags2)
    , profile(profile)
    , level(level)
    , parse_type(parse_type)
{

}

codec_params_t::codec_params_t(const std::string &codec_params)
    : codec_params_t()
{
    for (const auto& option : parse_option_list(codec_params))
    {
        switch(check_custom_param(option.first))
        {
            case custom_parameter_t::bitrate:
                bitrate = std::atoi(option.second.c_str());
            break;
            case custom_parameter_t::gop:
                gop = std::atoi(option.second.c_str());
            break;
            case custom_parameter_t::frame_size:
                frame_size = std::atoi(option.second.c_str());
            break;
            case custom_parameter_t::global_header:
                if (option.second.empty() || std::atoi(option.second.c_str()) != 0)
                {
                    flags1 |= AV_CODEC_FLAG_GLOBAL_HEADER;
                }
                else
                {
                    flags1 &= ~AV_CODEC_FLAG_GLOBAL_HEADER;
                }
            break;
            case custom_parameter_t::profile:
                profile = std::atoi(option.second.c_str());
            break;
            case custom_parameter_t::level:
                level = std::atoi(option.second.c_str());
            break;
            case custom_parameter_t::qmin:
                qmin = std::atoi(option.second.c_str());
            break;
            case custom_parameter_t::qmax:
                qmax = std::atoi(option.second.c_str());
            break;
        }

    }
}

bool codec_params_t::is_global_header() const
{
    return (flags1 & AV_CODEC_FLAG_GLOBAL_HEADER) != 0;
}

void codec_params_t::set_global_header(bool enable)
{
    flags1 = enable
            ? flags1 | AV_CODEC_FLAG_GLOBAL_HEADER
            : flags1 & ~AV_CODEC_FLAG_GLOBAL_HEADER;
}

void codec_params_t::load(const std::string &codec_params)
{
    for (const auto& option : parse_option_list(codec_params))
    {
        switch(check_custom_param(option.first))
        {
            case custom_parameter_t::bitrate:
                bitrate = std::atoi(option.second.c_str());
            break;
            case custom_parameter_t::gop:
                gop = std::atoi(option.second.c_str());
            break;
            case custom_parameter_t::frame_size:
                frame_size = std::atoi(option.second.c_str());
            break;
            case custom_parameter_t::global_header:
                if (option.second.empty() || std::atoi(option.second.c_str()) != 0)
                {
                    flags1 |= AV_CODEC_FLAG_GLOBAL_HEADER;
                }
                else
                {
                    flags1 &= ~AV_CODEC_FLAG_GLOBAL_HEADER;
                }
            break;
            case custom_parameter_t::profile:
                profile = std::atoi(option.second.c_str());
            break;
            case custom_parameter_t::level:
                level = std::atoi(option.second.c_str());
            break;
            case custom_parameter_t::qmin:
                qmin = std::atoi(option.second.c_str());
            break;
            case custom_parameter_t::qmax:
                qmax = std::atoi(option.second.c_str());
            break;
            default:;
        }

    }
}

std::string codec_params_t::to_params() const
{
    std::string result;

    if (bitrate != 0)
    {
        result.append(libav_param_name_bitrate);
        result.append("=");
        result.append(std::to_string(bitrate));
    }

    if (gop!= 0)
    {
        result.append(libav_param_name_gop);
        result.append("=");
        result.append(std::to_string(gop));
    }

    if (frame_size != 0)
    {
        result.append(libav_param_name_frame_size);
        result.append("=");
        result.append(std::to_string(frame_size));
    }

    if (is_global_header())
    {
        result.append(libav_param_name_bitrate);
        result.append("=1");
    }

    if (profile >= 0)
    {
        result.append(libav_param_name_profile);
        result.append("=");
        result.append(std::to_string(profile));
    }

    if (level >= 0)
    {
        result.append(libav_param_name_level);
        result.append("=");
        result.append(std::to_string(level));
    }
    if (qmin >= 0)
    {
        result.append(libav_param_name_qmin);
        result.append("=");
        result.append(std::to_string(qmin));
    }

    if (qmax >= 0)
    {
        result.append(libav_param_name_qmax);
        result.append("=");
        result.append(std::to_string(qmax));
    }

    return result;

}

bool is_registered()
{
    return libav_register_flag;
}

bool libav_register()
{
    if (!libav_register_flag)
    {
        avcodec_register_all();
        avdevice_register_all();
        avformat_network_init();
        av_register_all();
    }

    return libav_register_flag;
}

format_info_t frame_t::format_info() const
{
    switch(info.media_info.media_type)
    {
        case media_type_t::audio:
            return { info.media_info.audio_info.sample_format, info.codec_id };
        break;
        case media_type_t::video:
            return { info.media_info.video_info.pixel_format, info.codec_id };
        break;
        default:;
    }

    return {};
}


}
