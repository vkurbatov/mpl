#include "libav_output_format.h"
#include "libav_utils.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}


namespace ffmpeg
{

namespace detail
{

ff_const59 AVOutputFormat* find_output_format(const url_format_t& url_format)
{
    ff_const59 AVOutputFormat* format = nullptr;
    if (!url_format.format_type.empty())
    {
        format = av_guess_format(url_format.format_type.c_str()
                                 , nullptr
                                 , nullptr);
    }

    if (format != nullptr)
    {
        format = av_guess_format(nullptr
                                 , url_format.url.c_str()
                                 , nullptr);
    }

    return format;
}

bool add_stream(AVFormatContext* context
                , stream_info_t& stream_info)
{
    auto codec = avcodec_find_encoder(static_cast<AVCodecID>(stream_info.codec_info.id));

    if (codec != nullptr)
    {
        auto av_stream = avformat_new_stream(context
                                            , nullptr);

        if (av_stream != nullptr)
        {
            stream_info.stream_id = context->nb_streams - 1;
            (*av_stream) << stream_info;

            switch(av_stream->codecpar->codec_type)
            {
                case AVMEDIA_TYPE_AUDIO:
                    av_stream->time_base = { 1, av_stream->codecpar->sample_rate };
                break;
                case AVMEDIA_TYPE_VIDEO:
                    av_stream->time_base = { 1, static_cast<std::int32_t>(stream_info.media_info.video_info.fps) };
                break;
                case AVMEDIA_TYPE_DATA:

                break;
                case AVMEDIA_TYPE_SUBTITLE:

                break;
                default:;
            }

            if (stream_info.extra_data != nullptr)
            {
                av_stream->codecpar->extradata = stream_info.extra_data->data();
                av_stream->codecpar->extradata_size = stream_info.extra_data->size();
            }

            return true;
        }
    }

    return false;
}

}
using config_t = libav_output_format::config_t;

struct libav_output_format::context_t
{
    struct native_context_t
    {
        using u_ptr_t = std::unique_ptr<native_context_t>;

        struct AVFormatContext*     m_context;
        stream_info_t::list_t       m_streams;


        native_context_t()
            : m_context(nullptr)
        {

        }

        ~native_context_t()
        {
            close();
        }

        bool open(const config_t& config)
        {
            if (m_context == nullptr)
            {
                if (config.is_valid())
                {
                    url_format_t url_format = utils::fetch_url_format(config.url);
                    auto oformat = detail::find_output_format(url_format);

                    avformat_alloc_output_context2(&m_context
                                                   , oformat
                                                   , nullptr
                                                   , url_format.url.c_str());
                    if (m_context != nullptr)
                    {
                        m_streams.clear();

                        do
                        {

                            for (const auto& s : config.streams)
                            {
                                auto stream_info = s;
                                if (detail::add_stream(m_context
                                                       , stream_info))
                                {
                                    m_streams.emplace_back(std::move(stream_info));
                                }
                                else
                                {
                                    break;
                                }
                            }

                            if (m_streams.size() != config.streams.size())
                            {
                                break;
                            }

                            if ((m_context->oformat->flags & AVFMT_NOFILE) == 0)
                            {
                                if (avio_open(&m_context->pb
                                             , url_format.url.c_str()
                                             , AVIO_FLAG_WRITE) < 0)
                                {
                                    break;
                                }
                            }

                            if (m_context->oformat->flags & AVFMT_GLOBALHEADER)
                            {
                                if (av_write_trailer(m_context) < 0)
                                {
                                    break;
                                }
                            }

                            return true;

                        }
                        while(false);

                        close();
                    }
                }
            }
            return false;
        }

        bool close()
        {
            if (m_context != nullptr)
            {
                if (m_context->oformat->flags & AVFMT_GLOBALHEADER)
                {
                    av_write_trailer(m_context);
                }

                for (std::uint32_t i = 0; i < m_context->nb_streams; i++)
                {
                    m_context->streams[i]->codecpar->extradata = nullptr;
                    m_context->streams[i]->codecpar->extradata_size = 0;

                    if (m_context->streams[i]->codec != nullptr)
                    {
                        avcodec_free_context(&m_context->streams[i]->codec);
                    }
                    av_freep(&m_context->streams[i]);
                }

                if ((m_context->oformat->flags & AVFMT_NOFILE) == 0)
                {
                    avio_close(m_context->pb);
                }

                avformat_free_context(m_context);
                m_context = nullptr;

                return true;
            }

            return false;
        }

        bool is_open() const
        {
            return m_context != nullptr;
        }

        bool write(const frame_ref_t& frame)
        {
            if (m_context != nullptr)
            {
                std::uint32_t stream_id = static_cast<std::uint32_t>(frame.info.stream_id);
                if (stream_id < m_context->nb_streams)
                {
                    const stream_info_t& stream_info = m_streams[stream_id];
                    AVPacket av_packet = {};
                    auto& av_stream = *m_context->streams[stream_id];

                    if (frame.info.key_frame)
                    {
                        av_packet.flags |= AV_PKT_FLAG_KEY;
                    }

                    av_packet.stream_index = stream_id;
                    av_packet.data = const_cast<std::uint8_t*>(static_cast<const std::uint8_t*>(frame.data));
                    av_packet.size = frame.size;

                    switch(av_stream.codecpar->codec_type)
                    {
                        case AVMEDIA_TYPE_AUDIO:
                        {
                            av_packet.pts = frame.info.pts;
                            av_packet_rescale_ts(&av_packet
                                                 , { 1, static_cast<std::int32_t>(stream_info.media_info.audio_info.sample_rate) }
                                                 , av_stream.time_base);
                        }
                        break;
                        case AVMEDIA_TYPE_VIDEO:
                        {
                            av_packet.pts = frame.info.pts;
                            av_packet_rescale_ts(&av_packet
                                                 , { 1, 90000 }
                                                 , av_stream.time_base);
                        }
                        break;
                        default:;
                    }

                    av_packet.dts = AV_NOPTS_VALUE;

                    auto ret = av_interleaved_write_frame(m_context, &av_packet);

                    return ret >= 0;

                }
            }

            return false;
        }

    };

    config_t            m_config;
    native_context_t    m_native_context;

    using u_ptr_t = std::unique_ptr<context_t>;

    static u_ptr_t create(const config_t& config)
    {
        return std::make_unique<context_t>(config);
    }

    context_t(const config_t& config)
        : m_config(config)
    {

    }

    ~context_t()
    {
        close();
    }

    const config_t& config() const
    {
        return m_config;
    }

    bool set_config(const config_t& config)
    {
        if (!is_open())
        {
            m_config = config;
            return true;
        }

        return false;
    }

    bool open()
    {
        return m_native_context.open(m_config);
    }

    bool close()
    {
        return m_native_context.close();
    }

    bool is_open() const
    {
        return m_native_context.is_open();
    }

    bool write(const frame_ref_t &frame)
    {
        return m_native_context.write(frame);
    }
};

libav_output_format::config_t::config_t(const std::string_view &url
                                        , const std::string_view &options
                                        , const stream_info_t::list_t streams)
    : url(url)
    , options(options)
    , streams(streams)
{

}

bool libav_output_format::config_t::is_valid() const
{
    return !url.empty()
            && !streams.empty();
}

libav_output_format::libav_output_format(const config_t &config)
    : m_context(context_t::create(config))
{

}

libav_output_format::~libav_output_format()
{

}

const libav_output_format::config_t &libav_output_format::config() const
{
    return m_context->config();
}

bool libav_output_format::set_config(const config_t &config)
{
    return m_context->set_config(config);
}

bool libav_output_format::open()
{
    return m_context->open();
}

bool libav_output_format::close()
{
    return m_context->close();
}

bool libav_output_format::is_open() const
{
    return m_context->is_open();
}

bool libav_output_format::write(const frame_ref_t &frame)
{
    return m_context->write(frame);
}


}
