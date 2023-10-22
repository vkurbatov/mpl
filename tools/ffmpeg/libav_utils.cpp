#include "libav_utils.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include "tools/utils/url_base.h"


namespace pt::ffmpeg
{


namespace utils
{

libav_option_list_t parse_option_list(const std::string &options)
{
    return pt::utils::parse_option_list(options);
}

libav_option_map_t parse_option_map(const std::string &options)
{
    return pt::utils::parse_option_map(options);
}

bool is_global_header_format(const std::string &format_name)
{
    AVOutputFormat* format = av_guess_format(format_name.c_str()
                                             , nullptr
                                             , nullptr);

    return format != nullptr
            && (format->flags & AVFMT_GLOBALHEADER) != 0;
}

extra_data_t extract_global_header(const stream_info_t &stream_info)
{

    extra_data_t extra_data = nullptr;

    auto codec = avcodec_find_encoder(static_cast<AVCodecID>(stream_info.codec_info.id));

    if (codec != nullptr)
    {
        auto codec_context = avcodec_alloc_context3(codec);

        if (codec_context != nullptr)
        {
            codec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
            stream_info.media_info >> (*codec_context);

            switch(codec_context->codec_type)
            {
                case AVMEDIA_TYPE_AUDIO:
                {
                    if (codec_context->sample_fmt == AV_SAMPLE_FMT_NONE)
                    {
                        if (codec->sample_fmts != nullptr)
                        {
                            codec_context->sample_fmt = *codec->sample_fmts;
                        }
                    }
                }
                break;
                case AVMEDIA_TYPE_VIDEO:
                {
                    if (codec_context->pix_fmt == AV_PIX_FMT_NONE)
                    {
                        if (codec->pix_fmts != nullptr)
                        {
                            codec_context->pix_fmt = *codec->pix_fmts;
                        }
                    }
                }
                break;
            }

            if (avcodec_open2(codec_context
                              , codec
                              , nullptr) >= 0)
            {
                if (codec_context->extradata != nullptr
                        && codec_context->extradata_size > 0)
                {
                    extra_data = stream_info_t::create_extra_data(codec_context->extradata
                                                                  , codec_context->extradata_size
                                                                  , true);
                }

                avcodec_close(codec_context);
            }

            avcodec_free_context(&codec_context);
        }
    }

    return extra_data;
}

#define __merge_params(left, right) if ((right) > 0) (left) = (right); else (right) = (left)

void merge_codec_params(AVCodecContext &av_context
                        , codec_params_t &codec_params)
{
    __merge_params(av_context.bit_rate, codec_params.bitrate);
    __merge_params(av_context.gop_size, codec_params.gop);
    __merge_params(av_context.frame_size, codec_params.frame_size);
    __merge_params(av_context.profile, codec_params.profile);
    __merge_params(av_context.level, codec_params.level);
    __merge_params(av_context.qmin, codec_params.qmin);
    __merge_params(av_context.qmax, codec_params.qmax);
    av_context.flags |= codec_params.flags1;
    av_context.flags2 |= codec_params.flags2;
}

void set_options(AVDictionary** av_options
                 , const std::string &options)
{
    for (const auto& opt : parse_option_list(options))
    {
        av_dict_set(av_options, opt.first.c_str(), opt.second.c_str(), 0);
    }
}

void set_options(AVDictionary **av_options
                 , const libav_option_map_t &params)
{
    for (const auto& p : params)
    {
        av_dict_set(av_options, p.first.c_str(), p.second.c_str(), 0);
    }
}


device_type_t fetch_device_type(const std::string &uri)
{
    static const std::string device_names_table[] = { ""
                                                      , "rtsp://"
                                                      , "rtmp://"
                                                      , "rtp://"
                                                      , "v4l2://"
                                                      , "http://"
                                                      , "file://"
                                                      , "alsa://"
                                                      , "pulse://"
                                                     };

    if (uri.find("/") == 0)
    {
        return uri.find("/dev/video") == 0
                ? device_type_t::camera
                : device_type_t::file;
    }

    auto i = 0;
    for (const auto& device_name : device_names_table)
    {
        if (i > 0)
        {
            if (uri.find(device_name.c_str()) == 0)
            {
                return static_cast<device_type_t>(i);
            }
        }
        i++;
    }

    return device_type_t::unknown;
}

std::string error_string(int32_t av_errno)
{
    char serr[1024] = {};
    return av_make_error_string(serr, sizeof(serr), av_errno);
}

url_format_t fetch_url_format(const std::string &url)
{
    url_format_t format;
    pt::utils::url_info_t url_info;

    format.url = url;

    if (url_info.parse_url(url))
    {
        format.format_type = url_info.protocol;

        switch(fetch_device_type(url))
        {
            case device_type_t::alsa:
            case device_type_t::pulse:
            case device_type_t::camera:
                format.url = url_info.host;
            break;
            case device_type_t::rtmp:
                format.format_type = "flv";
            break;
            default:;
        }
    }

    return format;
}

}

AVCodecContext &operator <<(AVCodecContext &av_context
                            , const media_info_t& media_info)
{
    switch (media_info.media_type)
    {
        case media_type_t::audio:
            av_context.codec_type = AVMEDIA_TYPE_AUDIO;
            if (media_info.audio_info.sample_format != sample_format_none)
            {
                av_context.sample_fmt = static_cast<AVSampleFormat>(media_info.audio_info.sample_format);
            }
            av_context.sample_rate = media_info.audio_info.sample_rate;
            av_context.channels = media_info.audio_info.channels;
            av_context.channel_layout = av_context.channels > 1
                    ? AV_CH_LAYOUT_STEREO
                    : AV_CH_LAYOUT_MONO;
            av_context.time_base = { 1, av_context.sample_rate };
        break;

        case media_type_t::video:
            av_context.codec_type = AVMEDIA_TYPE_VIDEO;
            av_context.width = media_info.video_info.size.width;
            av_context.height = media_info.video_info.size.height;
            av_context.framerate = av_d2q(media_info.video_info.fps, max_fps);
            if (media_info.video_info.pixel_format != pixel_format_none)
            {
                av_context.pix_fmt = static_cast<AVPixelFormat>(media_info.video_info.pixel_format);
            }
            av_context.time_base = { 1, static_cast<std::int32_t>(media_info.video_info.fps) };
            av_context.sample_rate = video_sample_rate;
        break;
        case media_type_t::data:
            av_context.codec_type = AVMEDIA_TYPE_DATA;
        break;
    }

    return av_context;
}

AVCodecParameters &operator <<(AVCodecParameters &av_codecpar
                               , const media_info_t& media_info)
{
    switch (media_info.media_type)
    {
        case media_type_t::audio:
            av_codecpar.codec_type = AVMEDIA_TYPE_AUDIO;
            av_codecpar.format = media_info.audio_info.sample_format;
            av_codecpar.sample_rate = media_info.audio_info.sample_rate;
            av_codecpar.channels = media_info.audio_info.channels;
            av_codecpar.channel_layout = av_codecpar.channels > 1
                    ? AV_CH_LAYOUT_STEREO
                    : AV_CH_LAYOUT_MONO;
        break;

        case media_type_t::video:
            av_codecpar.codec_type = AVMEDIA_TYPE_VIDEO;
            av_codecpar.width = media_info.video_info.size.width;
            av_codecpar.height = media_info.video_info.size.height;
            // av_codecpar.sample_aspect_ratio = av_d2q(media_info.video_info.fps, max_fps);
            av_codecpar.format = media_info.video_info.pixel_format;
            av_codecpar.sample_rate = video_sample_rate;
        break;
        case media_type_t::data:
            av_codecpar.codec_type = AVMEDIA_TYPE_DATA;
        break;
    }

    return av_codecpar;
}

AVCodecContext &operator >>(const media_info_t &media_info, AVCodecContext &av_context)
{
    return av_context << media_info;
}

AVCodecParameters &operator >>(const media_info_t &media_info, AVCodecParameters &av_codecpar)
{
    return av_codecpar << media_info;
}

media_info_t &operator <<(media_info_t &media_info
                          , const AVCodecContext &av_context)
{
    switch (av_context.codec_type)
    {
        case AVMEDIA_TYPE_AUDIO:
            media_info.media_type = media_type_t::audio;
            media_info.audio_info.sample_format = av_context.sample_fmt;
            media_info.audio_info.sample_rate = av_context.sample_rate;
            media_info.audio_info.channels = av_context.channels;
        break;

        case AVMEDIA_TYPE_VIDEO:
            media_info.media_type = media_type_t::video;
            media_info.video_info.size.width= av_context.width;
            media_info.video_info.size.height = av_context.height;
            media_info.video_info.fps = av_q2d(av_context.framerate) + 0.5;
            media_info.video_info.pixel_format = av_context.pix_fmt;
        break;
        case AVMEDIA_TYPE_DATA:

        break;
    }

    return media_info;
}

media_info_t &operator <<(media_info_t &media_info
                          , const AVCodecParameters &av_codecpar)
{
    switch (av_codecpar.codec_type)
    {
        case AVMEDIA_TYPE_AUDIO:
            media_info.media_type = media_type_t::audio;
            media_info.audio_info.sample_format = av_codecpar.format;
            media_info.audio_info.sample_rate = av_codecpar.sample_rate;
            media_info.audio_info.channels = av_codecpar.channels;
        break;

        case AVMEDIA_TYPE_VIDEO:
            media_info.media_type = media_type_t::video;
            media_info.video_info.size.width= av_codecpar.width;
            media_info.video_info.size.height = av_codecpar.height;
            // media_info.video_info.fps = av_q2d(av_codecpar.sample_aspect_ratio) + 0.5;
            media_info.video_info.pixel_format = av_codecpar.format;
        break;
        case AVMEDIA_TYPE_DATA:
            media_info.media_type = media_type_t::data;
        break;
    }

    return media_info;
}

media_info_t &operator >>(const AVCodecContext &av_context
                          , media_info_t &media_info)
{
    return media_info << av_context;
}

media_info_t &operator >>(const AVCodecParameters &av_codecpar
                          , media_info_t &media_info)
{
    return media_info << av_codecpar;
}

AVStream& operator << (AVStream& av_stream
                       , const stream_info_t& stream_info)
{
    (*av_stream.codecpar) << stream_info.media_info;
    av_stream.index = stream_info.stream_id;
    av_stream.codecpar->codec_id = static_cast<AVCodecID>(stream_info.codec_info.id);

    av_stream.codecpar->bit_rate = stream_info.codec_info.codec_params.bitrate;
    av_stream.codecpar->frame_size = stream_info.codec_info.codec_params.frame_size;



    return av_stream;
}

stream_info_t& operator << (stream_info_t& stream_info
                            , const AVStream& av_stream)
{
    stream_info.media_info << *av_stream.codecpar;
    stream_info.stream_id = av_stream.index;

    switch(stream_info.media_info.media_type)
    {
        case media_type_t::video:
            stream_info.media_info.video_info.fps = av_q2d(av_stream.time_base) + 0.5;
        break;
        default:;
    }

    stream_info.codec_info.id = av_stream.codecpar->codec_id;
    stream_info.codec_info.name = codec_info_t::codec_name(stream_info.codec_info.id);

    if (stream_info.codec_info.id == codec_id_first_audio)
    {
        stream_info.codec_info.id = codec_id_none;
        stream_info.codec_info.name.clear();
    }

    stream_info.codec_info.codec_params.bitrate = av_stream.codecpar->bit_rate;
    stream_info.codec_info.codec_params.frame_size = av_stream.codecpar->frame_size;

    if (av_stream.codecpar->extradata != nullptr
            && av_stream.codecpar->extradata_size > 0)
    {
        stream_info.extra_data = stream_info_t::create_extra_data(av_stream.codecpar->extradata
                                                                  , av_stream.codecpar->extradata_size
                                                                  , true);
    }

    return stream_info;
}



}
