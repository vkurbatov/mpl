#ifndef FFMPEG_LIBAV_BASE_H
#define FFMPEG_LIBAV_BASE_H

#include "tools/base/frame_base.h"
#include "tools/base/time_base.h"
#include "tools/base/option_base.h"

#include <string>
#include <vector>
#include <queue>
#include <memory>
#include <functional>
#include <map>

namespace ffmpeg
{

using codec_id_t = std::int32_t;
using format_id_t = std::int32_t;
using pixel_format_t = format_id_t;
using sample_format_t = format_id_t;
using stream_parse_type_t = std::int32_t;

const std::int32_t default_frame_align = 1;

using pixel_formats_t = std::vector<pixel_format_t>;
using sample_formats_t = std::vector<sample_format_t>;

using format_list_t = std::vector<format_id_t>;

extern const std::int32_t padding_size;

constexpr codec_id_t unknown_codec_id = -1;
constexpr format_id_t unknown_format_id = -1;
constexpr pixel_format_t unknown_pixel_format = unknown_format_id;
constexpr sample_format_t unknown_sample_format = unknown_format_id;

extern const pixel_format_t pixel_format_none;
extern const pixel_format_t pixel_format_bgr8;
extern const pixel_format_t pixel_format_rgb8;
extern const pixel_format_t pixel_format_bgr15;
extern const pixel_format_t pixel_format_rgb15;
extern const pixel_format_t pixel_format_bgr16;
extern const pixel_format_t pixel_format_rgb16;
extern const pixel_format_t pixel_format_bgr24;
extern const pixel_format_t pixel_format_rgb24;
extern const pixel_format_t pixel_format_bgr32;
extern const pixel_format_t pixel_format_rgb32;
extern const pixel_format_t pixel_format_bgra;
extern const pixel_format_t pixel_format_rgba;
extern const pixel_format_t pixel_format_abgr;
extern const pixel_format_t pixel_format_argb;
extern const pixel_format_t pixel_format_gray8;
extern const pixel_format_t pixel_format_nv12;
extern const pixel_format_t pixel_format_nv21;
extern const pixel_format_t pixel_format_yuv420p;
extern const pixel_format_t pixel_format_yuv422p;

extern const pixel_format_t pixel_format_yuv444p;
extern const pixel_format_t pixel_format_yuv411p;
extern const pixel_format_t pixel_format_yuyv;
extern const pixel_format_t pixel_format_uyvy;
extern const pixel_format_t pixel_format_yuv410;

extern const pixel_format_t pixel_format_nv16;
extern const pixel_format_t pixel_format_bgr555;
extern const pixel_format_t pixel_format_bgr555x;
extern const pixel_format_t pixel_format_bgr565;
extern const pixel_format_t pixel_format_bgr565x;
extern const pixel_format_t pixel_format_rgb555;
extern const pixel_format_t pixel_format_rgb555x;
extern const pixel_format_t pixel_format_rgb565;
extern const pixel_format_t pixel_format_rgb565x;

extern const pixel_format_t pixel_format_abgr32;
extern const pixel_format_t pixel_format_argb32;
extern const pixel_format_t pixel_format_bgra32;
extern const pixel_format_t pixel_format_rgba32;

extern const pixel_format_t pixel_format_gray16;
extern const pixel_format_t pixel_format_gray16x;
extern const pixel_format_t pixel_format_sbggr8;
extern const pixel_format_t pixel_format_sgbrg8;
extern const pixel_format_t pixel_format_sgrbg8;
extern const pixel_format_t pixel_format_srggb8;

extern const sample_format_t sample_format_none;
extern const sample_format_t sample_format_pcm8;
extern const sample_format_t sample_format_pcm16;
extern const sample_format_t sample_format_pcm32;
extern const sample_format_t sample_format_float32;
extern const sample_format_t sample_format_float64;
extern const sample_format_t sample_format_pcm8p;
extern const sample_format_t sample_format_pcm16p;
extern const sample_format_t sample_format_pcm32p;
extern const sample_format_t sample_format_float32p;
extern const sample_format_t sample_format_float64p;


extern const stream_parse_type_t stream_parse_none;
extern const stream_parse_type_t stream_parse_full;
extern const stream_parse_type_t stream_parse_headers;
extern const stream_parse_type_t stream_parse_timestamp;
extern const stream_parse_type_t stream_parse_full_once;
extern const stream_parse_type_t stream_parse_full_raw;


extern const pixel_format_t default_pixel_format;
extern const sample_format_t default_sample_format;



extern const codec_id_t codec_id_flv1;
extern const codec_id_t codec_id_h261;
extern const codec_id_t codec_id_h263;
extern const codec_id_t codec_id_h263p;
extern const codec_id_t codec_id_h264;
extern const codec_id_t codec_id_h265;
extern const codec_id_t codec_id_vp8;
extern const codec_id_t codec_id_vp9;
extern const codec_id_t codec_id_mpeg4;
extern const codec_id_t codec_id_cpia;
extern const codec_id_t codec_id_jpeg;
extern const codec_id_t codec_id_mjpeg;
extern const codec_id_t codec_id_gif;
extern const codec_id_t codec_id_png;
extern const codec_id_t codec_id_raw_video;

extern const codec_id_t codec_id_aac;
extern const codec_id_t codec_id_opus;
extern const codec_id_t codec_id_pcma;
extern const codec_id_t codec_id_pcmu;

extern const codec_id_t codec_id_none;

enum class custom_parameter_t
{
    unknown,
    thread_count,
    bitrate,
    gop,
    frame_size,
    global_header,
    profile,
    level,
    qmin,
    qmax
};

extern const std::string libav_param_name_thread_count;
extern const std::string libav_param_name_bitrate;
extern const std::string libav_param_name_gop;
extern const std::string libav_param_name_frame_size;
extern const std::string libav_param_name_global_header;
extern const std::string libav_param_name_profile;
extern const std::string libav_param_name_level;
extern const std::string libav_param_name_qmin;
extern const std::string libav_param_name_qmax;


custom_parameter_t check_custom_param(const std::string param_name);


const std::uint32_t video_sample_rate = 90000;
const std::uint32_t max_fps = 60;
//extern const codec_id_t codec_id_yuv420p;

const std::size_t max_planes = 4;

using media_data_t = std::vector<std::uint8_t>;

using extra_data_t = std::shared_ptr<media_data_t>;

std::string error_to_string(std::int32_t av_error);

enum class media_type_t
{
    audio,
    video,
    data,
};

enum class streaming_event_t
{
    start,   
    stop,
    open,
    close
};

enum class device_type_t
{
    unknown,
    rtsp,
    rtmp,
    rtp,
    camera,
    http,
    file,
    alsa,
    pulse
};

enum stream_mask_t : std::uint32_t
{
    stream_mask_empty = 0,
    stream_mask_audio = (1 << 0),
    stream_mask_video = (1 << 1),
    stream_mask_data =  (1 << 2),
    stream_mask_only_media = stream_mask_audio | stream_mask_video,
    stream_mask_all = stream_mask_only_media | stream_mask_data
};

struct device_info_t;

using device_class_list_t = std::vector<std::string>;
using device_info_list_t = std::vector<device_info_t>;

struct device_info_t
{
    using list_t = std::vector<device_info_t>;
    static device_class_list_t device_class_list(media_type_t media_type
                                                 , bool is_source);
    static list_t device_list(media_type_t media_type
                               , bool is_source
                               , const std::string& device_class = {});

    media_type_t media_type;
    std::string name;
    std::string description;
    std::string device_class;
    bool        is_source;
    device_info_t(media_type_t media_type
                  , const std::string& name = {}
                  , const std::string& description = {}
                  , const std::string& device_class = {}
                  , bool is_source = true);

    std::string to_uri() const;

};

typedef std::uint32_t option_type_t;

enum class option_format_t
{
    numeric,
    real,
    string,
    unknown
};

using frame_point_t = base::frame_point_t;
using frame_size_t = base::frame_size_t;
using frame_rect_t = base::frame_rect_t;
using adaptive_timer_t = base::adaptive_timer_t;
using option_t = base::option_t;
using option_list_t = base::option_list_t;
const auto parse_option_list = base::parse_option_list;

struct audio_info_t
{
    std::uint32_t   sample_rate;
    std::uint32_t   channels;
    sample_format_t sample_format;

    static std::uint32_t bps(sample_format_t sample_format);
    static std::size_t sample_size(sample_format_t sample_format
                                   , std::uint32_t channels);
    static std::string format_name(sample_format_t sample_format);
    static bool is_planar(sample_format_t sample_format);

    audio_info_t(std::uint32_t sample_rate = 8000
                 , std::uint32_t channels = 1
                 , sample_format_t sample_format = default_sample_format);

    bool operator ==(const audio_info_t& audio_info) const;
    bool operator !=(const audio_info_t& audio_info) const;
    std::uint32_t bps() const;
    bool is_planar() const;
    std::size_t sample_size() const;
    std::string format_name() const;
};

typedef std::vector<frame_size_t> plane_sizes_t;

struct plane_info_t
{
    void* data;
    frame_size_t size;
};

typedef std::vector<plane_info_t> plane_list_t;

struct video_info_t
{
    frame_size_t    size;
    std::uint32_t   fps;
    pixel_format_t  pixel_format;

    static std::uint32_t bpp(pixel_format_t pixel_format);
    static std::size_t frame_size(pixel_format_t pixel_format
                                  , const frame_size_t& size
                                  , std::int32_t align = default_frame_align);
    static std::string format_name(pixel_format_t pixel_format);
    static std::size_t planes(pixel_format_t pixel_format);
    static std::size_t plane_width(pixel_format_t pixel_format
                                   , std::uint32_t width
                                   , std::uint32_t plane_idx);
    static plane_sizes_t plane_sizes(pixel_format_t pixel_format
                                     , const frame_size_t& size
                                     , std::int32_t align = default_frame_align);

    static std::size_t split_slices(pixel_format_t pixel_format
                                     , const frame_size_t& size
                                     , void *slices[max_planes]
                                     , const void* data
                                     , std::int32_t align = default_frame_align);

    static plane_list_t split_planes(pixel_format_t pixel_format
                                     , const frame_size_t& size
                                     , const void* data
                                     , std::int32_t align = default_frame_align);

    static bool blackout(pixel_format_t pixel_format
                                , const frame_size_t& size
                                , void *slices[max_planes]);

    static bool blackout(pixel_format_t pixel_format
                                , const frame_size_t& size
                                , void *data
                                , std::int32_t align = default_frame_align);


    static bool is_planar(pixel_format_t pixel_format);


    video_info_t(std::int32_t width
                 , std::int32_t height
                 , std::uint32_t fps = 1
                 , pixel_format_t pixel_format = default_pixel_format);

    video_info_t(frame_size_t size = { 0, 0 }
                 , std::uint32_t fps = 1
                 , pixel_format_t pixel_format = default_pixel_format);

    bool operator ==(const video_info_t& video_info) const;
    bool operator !=(const video_info_t& video_info) const;
    std::uint32_t bpp() const;
    std::size_t frame_size(std::int32_t align = default_frame_align) const;
    std::string format_name() const;

    std::size_t planes() const;
    std::size_t plane_width(std::uint32_t plane_idx) const;
    plane_sizes_t plane_sizes() const;
    std::size_t split_slices(void *slices[max_planes]
                             , const void* data
                             , std::int32_t align = default_frame_align) const;
    plane_list_t split_planes(const void* data = nullptr
                             , std::int32_t align = default_frame_align);

    bool blackout(void *slices[max_planes]) const;
    bool blackout(void *data
                  , std::int32_t align = default_frame_align) const;
};

struct fragment_info_t
{
    frame_rect_t    frame_rect;
    frame_size_t    frame_size;
    pixel_format_t  pixel_format;

    fragment_info_t(std::int32_t x
                    , std::int32_t y
                    , std::int32_t width
                    , std::int32_t height
                    , std::int32_t frame_width
                    , std::int32_t frame_height
                    , pixel_format_t pixel_format = default_pixel_format);

    fragment_info_t(const frame_rect_t& frame_rect = { 0, 0, 0, 0 }
                    , const frame_size_t& frame_size = { 0, 0 }
                    , pixel_format_t pixel_format = default_pixel_format);

    fragment_info_t(const frame_size_t& frame_size
                    , pixel_format_t pixel_format = default_pixel_format);

    bool operator ==(const fragment_info_t& fragment_info) const;
    bool operator !=(const fragment_info_t& fragment_info) const;

    std::size_t get_fragment_size(std::int32_t align = default_frame_align) const;
    std::size_t get_frame_size(std::int32_t align = default_frame_align) const;

    void adjust_align(const frame_size_t& align);

    bool is_full() const;
    bool is_convertable() const;
};


struct codec_params_t
{
    std::int32_t                bitrate;
    std::int32_t                gop;
    std::int32_t                frame_size;
    std::uint32_t               flags1;
    std::uint32_t               flags2;
    std::int32_t                profile;
    std::int32_t                level;
    stream_parse_type_t         parse_type;
    std::int32_t                qmin = -1;
    std::int32_t                qmax = -1;

    codec_params_t(std::int32_t bitrate = 0
                   , std::int32_t gop = 0
                   , std::int32_t frame_size = 0
                   , std::uint32_t flags1 = 0
                   , std::uint32_t flags2 = 0
                   , std::int32_t profile = -1
                   , std::int32_t level = -1
                   , stream_parse_type_t parse_type = stream_parse_full);

    codec_params_t(const std::string& codec_params);

    bool is_global_header() const;
    void set_global_header(bool enable);

    void load(const std::string& codec_params);

    std::string to_params() const;

};

struct codec_info_t
{
    codec_id_t                  id;
    std::string                 name;
    codec_params_t              codec_params;

    static std::string codec_name(codec_id_t id);
    static pixel_formats_t supported_video_formats(codec_id_t id);
    static sample_formats_t supported_audio_formats(codec_id_t id);
    static pixel_formats_t supported_video_formats(const std::string& name);
    static sample_formats_t supported_audio_formats(const std::string& name);

    codec_info_t(codec_id_t id = codec_id_none
                 , const std::string& name = ""
                 , const codec_params_t codec_params = codec_params_t());


    bool is_coded() const;
    std::string to_string() const;

    pixel_formats_t supported_video_formats() const;
    sample_formats_t supported_audio_formats() const;
};


struct media_info_t
{
    media_type_t                media_type;
    audio_info_t                audio_info;
    video_info_t                video_info;

    media_info_t() = default;
    media_info_t(const audio_info_t& audio_info);
    media_info_t(const video_info_t& video_info);

    bool operator==(const media_info_t& media_info) const;
    bool operator!=(const media_info_t& media_info) const;

    std::uint32_t sample_rate() const;

    std::string to_string() const;
};

struct frame_info_t
{
    media_info_t                media_info;
    std::int64_t                pts;
    std::int64_t                dts;
    std::int32_t                id;
    codec_id_t                  codec_id;
    bool                        key_frame;

    frame_info_t(const media_info_t& media_info = media_info_t()
                 , std::int64_t pts = 0
                 , std::int64_t dts = 0
                 , std::int32_t id = 0
                 , codec_id_t codec_id = codec_id_none
                 , bool key_frame = false);

    bool is_encoded() const;
    std::string to_string() const;
    std::int64_t timestamp() const;
    bool is_timestamp() const;
};

typedef std::int32_t stream_id_t;
const stream_id_t no_stream = -1;

struct format_info_t
{
    format_id_t     format_id;
    codec_id_t      codec_id;

    format_info_t(format_id_t format_id = unknown_format_id
                  , codec_id_t codec_id = unknown_codec_id);

    bool is_valid() const;

    bool is_encoded() const;
    bool is_convertable() const;
};

struct stream_info_t
{
    stream_id_t                 stream_id;
    codec_info_t                codec_info;
    media_info_t                media_info;
    extra_data_t                extra_data;

    static extra_data_t create_extra_data(const void* extra_data
                                          , std::size_t extra_data_size
                                          , bool need_padding = false);

    stream_info_t(stream_id_t stream_id = 0
                  , const codec_info_t& codec_info = codec_info_t()
                  , const media_info_t& media_info = media_info_t()
                  , const void* extra_data = nullptr
                  , std::size_t extra_data_size = 0
                  , bool need_extra_padding = false);

    format_info_t format_info() const;


    std::string to_string() const;

};

struct frame_t
{
    frame_info_t    info;
    media_data_t    media_data;
};

struct capture_diagnostic_t
{
    std::size_t     reconnections = 0;
    std::size_t     errors = 0;
    std::uint64_t   total_time = 0;
    std::uint64_t   alive_time = 0;
};

typedef std::vector<stream_info_t> stream_info_list_t;
typedef std::queue<frame_t> frame_queue_t;
typedef std::queue<media_data_t> media_queue_t;

typedef std::function<bool(const stream_info_t& stream_info
                           , frame_t&& frame)> frame_handler_t;

typedef std::function<bool(const stream_info_t& stream_info
                           , media_data_t&& media_data)> stream_data_handler_t;

typedef std::function<void(const streaming_event_t& streaming_event)> stream_event_handler_t;

bool is_registered();
bool libav_register();

}

#endif // ffmpeg_LIBAV_BASE_H
