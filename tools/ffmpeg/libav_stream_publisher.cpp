#include "libav_stream_publisher.h"
#include "libav_utils.h"

#include <thread>
//#include <mutex>
#include <map>
#include <atomic>
#include <chrono>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
//#include <libavdevice/avdevice.h>
#include <libavutil/opt.h>
}

#define WBS_MODULE_NAME "ff:publisher"
#include "tools/base/logger_base.h"

#include <iostream>
#include "tools/base/string_base.h"
#include "tools/base/url_base.h"

namespace ffmpeg
{

namespace detail
{

static const char* fetch_stream_name(device_type_t device_type)
{
    static const char* format_table[] =
    {
        nullptr     // unknown,
        , "rtsp"    // rtsp,
        , "flv"     // rtmp,
        , "rtp"     // rtp,
        , "v4l2"    // camera,
        , nullptr   // http,
        , "mpeg"    // file,
        , "alsa"    // alsa
        , "pulse"   // pulse
    };

    return format_table[static_cast<std::int32_t>(device_type)];
}

ff_const59 AVOutputFormat* find_output_format(const std::string& url)
{
    portable::url_info_t url_info;

    if (url_info.parse_url(url))
    {
        switch(utils::fetch_device_type(url))
        {
            case device_type_t::rtsp:
            case device_type_t::rtmp:
            case device_type_t::rtp:
            case device_type_t::http:
            case device_type_t::file:
                return av_guess_format(nullptr
                                       , url.c_str()
                                       , nullptr);
            break;
            case device_type_t::camera:
            case device_type_t::alsa:
            case device_type_t::pulse:
                return av_guess_format(url_info.protocol.c_str()
                                       , nullptr
                                       , nullptr);
            break;
            default:;
        }
    }
    return nullptr;
}

}

struct libav_output_format_context_t
{   
    struct AVFormatContext*     context;
    stream_info_list_t          streams;
    bool                        is_init;
    std::string                 uri;
    device_type_t               device_type;
    std::int64_t                audio_pts;
    std::int64_t                video_pts;
    std::int64_t                audio_ts;
    std::int64_t                video_ts;

    libav_output_format_context_t(const std::string& uri
                                  , const stream_info_list_t& stream_list)
        : context(nullptr)
        , is_init(false)
        , uri(uri)
        , device_type(utils::fetch_device_type(uri))
        , audio_pts(0)
        , video_pts(0)
        , audio_ts(0)
        , video_ts(0)
    {
        is_init = init(uri
                       , stream_list);


    }

    ~libav_output_format_context_t()
    {
        if (context != nullptr)
        {
            if (is_init)
            {
                av_write_trailer(context);
            }


            for (auto i = 0; i < static_cast<std::int32_t>(context->nb_streams); i++)
            {
                context->streams[i]->codecpar->extradata = nullptr;
                context->streams[i]->codecpar->extradata_size = 0;

                if (context->streams[i]->codec != nullptr)
                {
                    avcodec_free_context(&context->streams[i]->codec);
                }
                av_freep(&context->streams[i]);
            }

            if ((context->oformat->flags & AVFMT_NOFILE) == 0)
            {
                avio_close(context->pb);
            }

            avformat_free_context(context);
        }
    }

    bool init(const std::string& uri
              , const stream_info_list_t& stream_list)
    {

        /*auto format = av_guess_format(nullptr
                                      , uri.c_str()
                                      , nullptr);*/

        auto format = detail::find_output_format(uri);

        /*if (device_type == device_type_t::file)
        {
            format = nullptr;
        }*/

        auto c_url = uri.c_str();

        switch(device_type)
        {
            case device_type_t::alsa:
                c_url += 7;
            break;
            case device_type_t::pulse:
                c_url += 8;
            break;
            default:;
        }


        avformat_alloc_output_context2(&context
                                        , format
                                        , format == nullptr ? detail::fetch_stream_name(device_type) : nullptr
                                        , c_url);

        if (context != nullptr)
        {
            for(const auto& strm: stream_list)
            {
                add_stream(strm);
            }

            if (context->nb_streams > 0)
            {
                return finish_init();
            }
        }


        return false;
    }

    bool add_stream(const stream_info_t& stream_info)
    {
        auto codec = avcodec_find_encoder(static_cast<AVCodecID>(stream_info.codec_info.id));

        //if (codec != nullptr)
        {
            auto av_stream = avformat_new_stream(context
                                                , nullptr);

            if (av_stream != nullptr)
            {
                auto strm = stream_info;

                strm.stream_id = context->nb_streams - 1;

                if ((context->oformat->flags & AVFMT_GLOBALHEADER) != 0)
                {
                    strm.codec_info.codec_params.set_global_header(true);
                }

                av_stream->id = strm.stream_id;
                av_stream->codecpar->bit_rate = strm.codec_info.codec_params.bitrate;
                av_stream->codecpar->codec_id = static_cast<AVCodecID>(strm.codec_info.id);
                av_stream->codecpar->frame_size = strm.codec_info.codec_params.frame_size;

                if (codec != nullptr)
                {
                    av_stream->codecpar->codec_type = codec->type;
                }
                else
                {
                    switch(stream_info.media_info.media_type)
                    {
                        case media_type_t::audio:
                            av_stream->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
                        break;
                        case media_type_t::video:
                            av_stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
                        break;
                        case media_type_t::data:
                            av_stream->codecpar->codec_type = AVMEDIA_TYPE_DATA;
                        break;
                    }
                }

                switch (av_stream->codecpar->codec_type)
                {
                    case AVMEDIA_TYPE_AUDIO:
                        strm.extra_data = nullptr;
                        if (strm.media_info.audio_info.sample_format == sample_format_none)
                        {
                            strm.media_info.audio_info.sample_format = strm.codec_info.id == codec_id_aac
                                    ? sample_format_float32p
                                    : sample_format_pcm16;
                        }
                        strm.media_info >> *(av_stream->codecpar);
                        context->oformat->audio_codec = av_stream->codecpar->codec_id;

                        av_stream->time_base = { 1, av_stream->codecpar->sample_rate };
                        //strm.codec_info.codec_params.set_global_header(false);

                        if ((strm.codec_info.codec_params.is_global_header())
                                && strm.extra_data == nullptr)
                        {
                            strm.extra_data = utils::extract_global_header(strm);
                        }

                    break;
                    case AVMEDIA_TYPE_VIDEO:
                        if (strm.media_info.video_info.pixel_format == pixel_format_none)
                        {
                            strm.media_info.video_info.pixel_format = pixel_format_yuv420p;
                        }
                        strm.media_info >> *(av_stream->codecpar);
                        context->oformat->video_codec = av_stream->codecpar->codec_id;

                        av_stream->time_base = { 1, static_cast<std::int32_t>(stream_info.media_info.video_info.fps) };
                        av_stream->sample_aspect_ratio = av_stream->codecpar->sample_aspect_ratio;
                        //av_stream->display_aspect_ratio = av_stream->sample_aspect_ratio;
                        av_stream->r_frame_rate = av_stream->codecpar->sample_aspect_ratio;

                        if ((strm.codec_info.codec_params.is_global_header())
                                && strm.extra_data == nullptr)
                        {
                            strm.extra_data = utils::extract_global_header(strm);
                        }

                    break;
                    case AVMEDIA_TYPE_DATA:
                        context->oformat->data_codec = av_stream->codecpar->codec_id;
                    break;
                    case AVMEDIA_TYPE_SUBTITLE:
                        context->oformat->subtitle_codec = av_stream->codecpar->codec_id;
                    break;
                }

                if (strm.extra_data != nullptr)
                {
                    av_stream->codecpar->extradata = strm.extra_data->data();
                    av_stream->codecpar->extradata_size = strm.extra_data->size();// - AV_INPUT_BUFFER_PADDING_SIZE;
                }

                streams.emplace_back(std::move(strm));
                return true;
            }

        }
        return false;
    }

    bool finish_init()
    {
        av_dump_format(context, 0, uri.c_str(), 1);

        if ((context->oformat->flags & AVFMT_NOFILE) == 0)
        {
            if (avio_open(&context->pb, uri.c_str(), AVIO_FLAG_WRITE) < 0)
            {
                return false;
            }
        }


        if (device_type != device_type_t::alsa
                && device_type != device_type_t::pulse)
        {

            auto res = avformat_write_header(context
                                             , nullptr);

            return res >= 0;
        }

        return true;
    }

    bool push_frame(std::int32_t stream_id
                    , const void* data
                    , std::size_t size
                    , bool key_frame
                    , std::int64_t timestamp)
    {
        if (stream_id >= 0
                && stream_id < static_cast<std::int32_t>(context->nb_streams))
        {
            const stream_info_t& s_info = streams[stream_id];

            AVPacket av_packet = {};

            auto& av_stream = *context->streams[stream_id];

            if (key_frame)
            {
                av_packet.flags |= AV_PKT_FLAG_KEY;
            }


            av_packet.stream_index = stream_id;
            av_packet.data = const_cast<std::uint8_t*>(static_cast<const std::uint8_t*>(data));      
            av_packet.size = size;

            switch(av_stream.codecpar->codec_type)
            {
                case AVMEDIA_TYPE_AUDIO:
                {
                    if (audio_ts == 0)
                    {
                        audio_pts = 0;
                    }
                    else
                    {
                        auto dt = timestamp - audio_ts;
                        if (std::abs(dt) > s_info.media_info.audio_info.sample_rate)
                        {
                            dt = 0;
                        }
                        audio_pts += dt;
                    }

                    audio_ts = timestamp;

                    av_packet.pts = audio_pts;

                    av_packet_rescale_ts(&av_packet
                                         , { 1, static_cast<std::int32_t>(s_info.media_info.audio_info.sample_rate) }
                                         , av_stream.time_base);
                }
                break;
                case AVMEDIA_TYPE_VIDEO:
                {
                    if (video_ts == 0)
                    {
                        video_pts = 0;
                    }
                    else
                    {
                        auto dt = timestamp - video_ts;
                        if (std::abs(dt) > 90000)
                        {
                            dt = 0;
                        }
                        video_pts += dt;
                    }

                    video_ts = timestamp;
                    av_packet.pts = video_pts;
                    av_packet_rescale_ts(&av_packet
                                         , { 1, 90000 }
                                         , av_stream.time_base);
                }
                break;
            }

            av_packet.dts = AV_NOPTS_VALUE;

            LOG_D << "WRITE STREAM [" << stream_id << "] PACKET SIZE: " << size << ". pts = " << av_packet.pts << ", flags = " << av_packet.flags LOG_END;

            auto ret = av_interleaved_write_frame(context, &av_packet);

            return ret >= 0;
        }

        return false;
    }
};

struct libav_stream_publisher_context_t
{

    std::unique_ptr<libav_output_format_context_t> m_format_context;

    libav_stream_publisher_context_t()
    {

    }

    bool open(const std::string& uri
              , const stream_info_list_t& stream_list)
    {

        m_format_context.reset(new libav_output_format_context_t(uri
                                                                 , stream_list));

        if (m_format_context->is_init)
        {
            return true;
        }

        m_format_context.reset();

        return false;
    }

    bool close()
    {
        if (m_format_context != nullptr)
        {
            m_format_context.reset(nullptr);
            return true;
        }

        return false;
    }
    bool is_opened() const
    {
        return m_format_context != nullptr;
    }
    bool is_established() const
    {
        return is_opened();
    }

    stream_info_list_t streams() const
    {
        if (m_format_context != nullptr)
        {
            return m_format_context->streams;
        }

        return stream_info_list_t();
    }

    bool push_frame(std::int32_t stream_id
                    , const void* data
                    , std::size_t size
                    , bool key_frame
                    , std::int64_t timestamp)
    {
        return m_format_context->push_frame(stream_id
                                            , data
                                            , size
                                            , key_frame
                                            , timestamp);
    }
};
//--------------------------------------------------------------------------
void libav_stream_publisher_context_deleter_t::operator()(libav_stream_publisher_context_t *libav_stream_publisher_context_ptr)
{
    delete libav_stream_publisher_context_ptr;
}
//--------------------------------------------------------------------------
libav_stream_publisher::libav_stream_publisher()
 : m_libav_stream_publisher_context(new libav_stream_publisher_context_t())
{

}

bool libav_stream_publisher::open(const std::string &uri
                                  , const stream_info_list_t &stream_list)
{
    return m_libav_stream_publisher_context->open(uri
                                                  , stream_list);
}

bool libav_stream_publisher::close()
{
    return m_libav_stream_publisher_context->close();
}

bool libav_stream_publisher::is_opened() const
{
    return m_libav_stream_publisher_context->is_opened();
}

bool libav_stream_publisher::is_established() const
{
    return m_libav_stream_publisher_context->is_established();
}

stream_info_list_t libav_stream_publisher::streams() const
{
    return m_libav_stream_publisher_context->streams();
}

bool libav_stream_publisher::push_frame(int32_t stream_id
                                        , const void *data
                                        , std::size_t size
                                        , std::int64_t timestamp
                                        , bool key_frame)
{
    return m_libav_stream_publisher_context->push_frame(stream_id
                                                        , data
                                                        , size
                                                        , key_frame
                                                        , timestamp);
}

bool libav_stream_publisher::push_frame(const frame_t& frame)
{
    return m_libav_stream_publisher_context->push_frame(frame.info.stream_id
                                                        , frame.media_data.data()
                                                        , frame.media_data.size()
                                                        , frame.info.key_frame
                                                        , frame.info.timestamp());
}

}
