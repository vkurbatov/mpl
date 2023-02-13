#include "libav_input_format.h"
#include "libav_utils.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libavutil/opt.h>
}

#define WBS_MODULE_NAME "ff:input_fmt"
#include "tools/base/logger_base.h"

namespace ffmpeg
{

typedef std::map<stream_id_t, stream_info_t> stream_map_t;

struct native_input_format_context_t
{
    struct AVFormatContext*     context;
    struct AVPacket             packet;
    std::uint32_t               context_id;
    std::size_t                 total_read_bytes;
    std::size_t                 total_read_frames;
    device_type_t               type;

    stream_map_t                streams;

    bool                        is_init;
    bool                        is_established;

    native_input_format_context_t(const std::string& uri
                                  , const std::string& options)
        : context(nullptr)
        , context_id(0)
        , total_read_bytes(0)
        , total_read_frames(0)
        , type(device_type_t::unknown)
        , is_init(false)
        , is_established(false)
    {
        static std::uint32_t ctx_id = 0;
        context_id = ctx_id++;

        av_init_packet(&packet);

        auto result = init(uri
                           , options) >= 0;

        LOG_T << "Context #" << context_id << ". Create with result = " << result LOG_END;
    }
    ~native_input_format_context_t()
    {
        LOG_T << "Context #" << context_id << ". Destroy" LOG_END;

        av_packet_unref(&packet);

        if (context != nullptr)
        {
            // av_read_pause(context);
            avformat_close_input(&context);
            avformat_free_context(context);

            LOG_I << "Context #" << context_id << ". Free resources" LOG_END;
        }
    }

    std::int32_t init(const std::string& uri
                      , const std::string& options)
    {
        std::int32_t result = -1;

        auto c_uri = uri.c_str();

        auto device_type = utils::fetch_device_type(uri);

        if (!is_init)
        {
            AVDictionary* av_options = nullptr;

            type = device_type;
            switch(type)
            {
                case device_type_t::rtsp:
                    av_dict_set_int(&av_options, "stimeout", 1000000, 0);
                break;
                case device_type_t::camera:
                    av_dict_set(&av_options, "pixel_format", "mjpeg", 0);
                    c_uri += 6;
                    if (*c_uri != '/')
                    {
                        c_uri++;
                    }
                break;
            }

            for (const auto& o: parse_option_list(options))
            {
                 av_dict_set(&av_options, o.first.c_str(), o.second.c_str(), 0);
            }

            result = avformat_open_input(&context
                                         , c_uri
                                         , nullptr
                                         , &av_options);

            if (result == 0)
            {
                result = avformat_find_stream_info(context
                                                   , nullptr);

                is_init = result >= 0;

                if (is_init)
                {
                    LOG_I << "Context #" << context_id << ". Open streams (" << context->nb_streams << ") success" LOG_END;
                    for (const auto& s : get_streams())
                    {
                        streams.insert(std::make_pair(s.stream_id, s));
                    }
                }
                else
                {
                    LOG_E << "Context #" << context_id << ". Open streams failed, err = " << result LOG_END;
                }
            }
        }

        return result;
    }

    stream_info_list_t get_streams(stream_mask_t stream_mask = stream_mask_t::stream_mask_all)
    {
        stream_info_list_t streams;

        bool is_audio_allowed = (stream_mask & stream_mask_t::stream_mask_audio) != stream_mask_empty;
        bool is_video_allowed = (stream_mask & stream_mask_t::stream_mask_video) != stream_mask_empty;
        bool is_data_allowed = (stream_mask & stream_mask_t::stream_mask_data) != stream_mask_empty;

        if (is_init)
        {
            for (unsigned i = 0; i < context->nb_streams; i++)
            {
                auto av_stream = context->streams[i];

                stream_info_t stream_info = {};

                switch(av_stream->codec->codec_type)
                {
                    case AVMEDIA_TYPE_AUDIO:

                        if (!is_audio_allowed)
                        {
                            continue;
                        }

                        stream_info.media_info.media_type = media_type_t::audio;

                        stream_info.media_info.audio_info.sample_rate = av_stream->codec->sample_rate;
                        stream_info.media_info.audio_info.channels = av_stream->codec->channels;
                        stream_info.media_info.audio_info.sample_format = static_cast<sample_format_t>(av_stream->codec->sample_fmt);

                    break;
                    case AVMEDIA_TYPE_VIDEO:

                        if (!is_video_allowed)
                        {
                            continue;
                        }

                        stream_info.media_info.media_type = media_type_t::video;

                        stream_info.media_info.video_info.size.width = av_stream->codec->width;
                        stream_info.media_info.video_info.size.height = av_stream->codec->height;

                        stream_info.media_info.video_info.fps = av_q2d(av_stream->avg_frame_rate) + 0.5;

                        if (stream_info.media_info.video_info.fps == 0)
                        {
                            stream_info.media_info.video_info.fps = av_q2d(av_stream->r_frame_rate) + 0.5;
                        }

                        stream_info.media_info.video_info.pixel_format = static_cast<pixel_format_t>(av_stream->codec->pix_fmt);
                        stream_info.codec_info.codec_params.gop = av_stream->codec->gop_size;
                    break;

                    case AVMEDIA_TYPE_DATA:
                    case AVMEDIA_TYPE_SUBTITLE:

                        if (!is_data_allowed)
                        {
                            continue;
                        }
                        stream_info.media_info.media_type = media_type_t::data;
                    break;
                    default:
                        continue;
                }

                stream_info.stream_id = av_stream->index;
                if (av_stream->codec != nullptr)
                {
                    stream_info.codec_info.id = av_stream->codec->codec_id;
                    stream_info.codec_info.name = stream_info.codec_info.codec_name(stream_info.codec_info.id);

                    if (stream_info.codec_info.id == codec_id_first_audio)
                    {
                        stream_info.codec_info.id = codec_id_none;
                        stream_info.codec_info.name.clear();
                    }
                    stream_info.codec_info.codec_params.bitrate = av_stream->codec->bit_rate;
                    stream_info.codec_info.codec_params.frame_size = av_stream->codec->frame_size;
                    stream_info.codec_info.codec_params.flags1 = av_stream->codec->flags;
                    stream_info.codec_info.codec_params.flags2 = av_stream->codec->flags2;
                }

                if (av_stream->codec->extradata != nullptr
                        && av_stream->codec->extradata_size > 0)
                {
                    stream_info.extra_data = std::move(stream_info_t::create_extra_data(av_stream->codec->extradata
                                                                                        , av_stream->codec->extradata_size
                                                                                        , true));
                }

                streams.emplace_back(std::move(stream_info));
            }
        }

        return streams;
    }

    std::int32_t fetch_media_data(frame_t& frame)
    {
        std::int32_t result = -1;

        if (is_init)
        {
            result = av_read_frame(context, &packet);

            if (result >= 0 && packet.size > 0)
            {
                frame.info.dts = packet.dts;
                frame.info.dts = packet.pts;
                frame.info.id = packet.stream_index;
                frame.info.key_frame = (packet.flags & AV_PKT_FLAG_KEY) != 0;

                frame.media_data.resize(packet.size);

                memcpy(frame.media_data.data()
                            , packet.data
                            , packet.size);

                total_read_bytes += packet.size;
                total_read_frames++;

                result = packet.stream_index;

                auto it = streams.find(result);

                if (it != streams.end())
                {
                    frame.info.codec_id = it->second.codec_info.id;
                    frame.info.media_info = it->second.media_info;
                }
            }


            LOG_D << "Context #" << context_id << ". Fetch media data size " << packet.size
                  << " from stream #" << result LOG_END;

            av_packet_unref(&packet);
        }
        else
        {
            LOG_W << "Context #" << context_id << ". Cant't fetch media data, context not init" LOG_END;
        }

        is_established = result >= 0;

        return result;
    }

};

struct libav_input_format_context_t
{
    typedef std::unique_ptr<native_input_format_context_t> native_format_context_ptr_t;
    native_format_context_ptr_t     m_native_input_format_context;

    libav_input_format_context_t()
    {

    }

    bool open(const std::string &uri
              , const std::string &options)
    {
        close();
        m_native_input_format_context.reset(new native_input_format_context_t(uri
                                                                              , options));

        if (m_native_input_format_context->is_init)
        {
            return true;
        }

        m_native_input_format_context.reset();

        return false;
    }

    bool close()
    {
        if (m_native_input_format_context != nullptr)
        {
            m_native_input_format_context.reset();
            return true;
        }
        return false;
    }

    bool is_opened() const
    {
        return m_native_input_format_context != nullptr;
    }

    bool is_established() const
    {
        return m_native_input_format_context != nullptr
                && m_native_input_format_context->is_established;
    }

    stream_info_list_t streams() const
    {
        if (m_native_input_format_context != nullptr)
        {
            return m_native_input_format_context->get_streams();
        }

        return { };
    }

    bool fetch_frame(frame_t &frame)
    {
        return (m_native_input_format_context != nullptr)
                         && m_native_input_format_context->fetch_media_data(frame) >= 0;
    }
};
// ---------------------------------------------------------------------
void libav_input_format_context_deleter_t::operator()(libav_input_format_context_t *libav_input_format_context_ptr)
{
    delete libav_input_format_context_ptr;
}
// ---------------------------------------------------------------------
libav_input_format::libav_input_format()
    : m_libav_input_format_context(new libav_input_format_context_t())
{

}

bool libav_input_format::open(const std::string &uri
                              , const std::string &options)
{
    return m_libav_input_format_context->open(uri
                                              , options);
}

bool libav_input_format::close()
{
    return m_libav_input_format_context->close();
}

bool libav_input_format::is_opened() const
{
    return m_libav_input_format_context->is_opened();
}

bool libav_input_format::is_established() const
{
    return m_libav_input_format_context->is_established();
}

stream_info_list_t libav_input_format::streams() const
{
    return m_libav_input_format_context->streams();
}

bool libav_input_format::fetch_frame(frame_t &frame)
{
    return m_libav_input_format_context->fetch_frame(frame);
}


}
