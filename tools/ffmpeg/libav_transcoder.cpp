#include "libav_transcoder.h"
#include "libav_utils.h"

#include <cstring>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
}

#define WBS_MODULE_NAME "ff:transcoder"
#include "tools/base/logger_base.h"

#include <map>
#include <limits>

#include <iostream>
#include "tools/base/string_base.h"

namespace ffmpeg
{

namespace utils
{

template<typename T>
T max_val()
{
    return std::numeric_limits<T>::max();
}

template<> float max_val()
{
    return 1.0f;
}
template<> double max_val()
{
    return 1.0;
}

template<typename Tin, typename Tout = std::int16_t>
void sample_convert(const void* input_buffer
                    , void* output_buffer
                    , std::size_t samples
                    , std::int32_t step = 1)
{
    auto input_ptr = reinterpret_cast<const Tin*>(input_buffer);
    auto output_ptr = reinterpret_cast<Tout*>(output_buffer);

    while (samples-- > 0)
    {
        //*output_ptr = (*input_ptr * max_val<Tout>()) / static_cast<Tout>(max_val<Tin>());
        *output_ptr = static_cast<Tout>((static_cast<double>(*input_ptr) * static_cast<double>(max_val<Tout>())) / static_cast<double>(max_val<Tin>()));

        output_ptr += step > 0 ? step : 1;
        input_ptr += step < 0 ? -step : 1 ;
    }
}


void update_context_info(const AVCodecContext& av_context
                         , stream_info_t& stream_info
                         , AVFrame& av_frame)
{

    stream_info.codec_info.id = av_context.codec->id;
    stream_info.codec_info.name = av_context.codec->name;
    stream_info.codec_info.codec_params.bitrate = av_context.bit_rate;
    stream_info.codec_info.codec_params.frame_size = av_context.frame_size;
    stream_info.codec_info.codec_params.flags1 = av_context.flags;
    stream_info.codec_info.codec_params.flags2 = av_context.flags2;

    stream_info.media_info << av_context;

    if (av_context.codec_type == AVMEDIA_TYPE_AUDIO)
    {
        av_frame.channels = av_context.channels;
        av_frame.channel_layout = av_context.channel_layout;
        av_frame.format = av_context.sample_fmt;
    }
    else
    {

        stream_info.codec_info.codec_params.gop = av_context.gop_size;

        av_frame.width = av_context.width;
        av_frame.height = av_context.height;
        av_frame.sample_aspect_ratio = av_context.time_base;
        av_frame.format = av_context.pix_fmt;
    }

    if (av_context.extradata != nullptr
            && av_context.extradata_size > 0)
    {

        stream_info.extra_data = stream_info_t::create_extra_data(av_context.extradata
                                                                  , av_context.extradata_size
                                                                  , true);
    }

    av_frame.sample_rate = av_context.sample_rate;
    av_frame.pts = AV_NOPTS_VALUE;
}

bool set_custom_option(AVCodecContext& av_context
                          , const option_t& option)
{
    switch(check_custom_param(option.first))
    {
        case custom_parameter_t::thread_count:
            av_context.thread_count = std::atoi(option.second.c_str());
        break;
        case custom_parameter_t::bitrate:
            av_context.bit_rate = std::atoi(option.second.c_str());
        break;
        case custom_parameter_t::gop:
            av_context.gop_size = std::atoi(option.second.c_str());
        break;
        case custom_parameter_t::frame_size:
            av_context.frame_size = std::atoi(option.second.c_str());
        break;
        case custom_parameter_t::global_header:
            if (option.second.empty() || std::atoi(option.second.c_str()) != 0)
            {
                av_context.flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
            }
            else
            {
                av_context.flags &= ~AV_CODEC_FLAG_GLOBAL_HEADER;
            }
        break;
        case custom_parameter_t::profile:
            av_context.profile = std::atoi(option.second.c_str());
        break;
        case custom_parameter_t::level:
            av_context.level = std::atoi(option.second.c_str());
        break;
        case custom_parameter_t::qmin:
            av_context.qmin = std::atoi(option.second.c_str());
        break;
        case custom_parameter_t::qmax:
            av_context.qmax = std::atoi(option.second.c_str());
        break;
        default:
            return false;
    }
    return true;
}

void set_extended_options(AVCodecContext& av_context
                          , AVDictionary **av_options
                          , const std::string& options)
{
    for (const auto& opt : parse_option_list(options))
    {
        if (!set_custom_option(av_context
                               , opt))
        {
            av_dict_set(av_options, opt.first.c_str(), opt.second.c_str(), 0);
        }
    }
}

AVCodec* get_codec(const codec_info_t& codec_info
                   , bool is_encoder)
{
    AVCodec* codec = nullptr;

    auto get_codec_by_name = is_encoder
             ? &avcodec_find_encoder_by_name
             : &avcodec_find_decoder_by_name;

    auto get_codec_by_id = is_encoder
             ? &avcodec_find_encoder
             : &avcodec_find_decoder;

    if (!codec_info.name.empty())
    {
        codec = get_codec_by_name(codec_info.name.c_str());
    }

    if (codec == nullptr)
    {
        codec = get_codec_by_id(static_cast<AVCodecID>(codec_info.id));
    }

    return codec;
}

}

static bool operator & (transcode_flag_t lfl, transcode_flag_t rfl)
{
    return (static_cast<std::uint32_t>(lfl) & static_cast<std::uint32_t>(rfl)) != 0;
}


static std::uint32_t g_context_id = 0;

struct libav_codec_context_t
{
    using u_ptr_t = std::unique_ptr<libav_codec_context_t>;

    struct AVCodecContext*      av_context;
    struct AVFrame              av_frame;
    struct AVPacket             av_packet;
    media_data_t                resample_buffer;
    std::uint32_t               context_id;
    std::int32_t                frame_counter;
    bool                        is_encoder;
    bool                        is_init;
    media_data_t                audio_buffer;

    libav_codec_context_t(stream_info_t& stream_info
                          , bool is_encoder
                          , const std::string& options)
        : av_context(nullptr)
        , av_frame{}
        , av_packet{}
        , context_id(++g_context_id)
        , frame_counter(0)
        , is_encoder(is_encoder)
        , is_init(false)
    {
        is_init = init(stream_info
                       , options);
    }

    ~libav_codec_context_t()
    {
        LOG_D << "Transcoder #" << context_id << ". Destroy transcoder" LOG_END;

        reset();
    }

    void reset()
    {
        is_init = false;

        if (av_context != nullptr)
        {
            av_context->extradata = nullptr;
            av_context->extradata_size = 0;

            if (avcodec_is_open(av_context) > 0)
            {
                avcodec_close(av_context);
            }

            avcodec_free_context(&av_context);
            LOG_I << "Transcoder #" << context_id << ". Free context resource success" LOG_END;


            av_context = nullptr;
            av_frame = {};
            av_packet = {};
        }
    }


    bool reinit(stream_info_t& stream_info
                , bool is_encoder
                , const std::string& options)
    {
        reset();
        this->is_encoder = is_encoder;
        is_init = init(stream_info
                       , options);

        return is_init;
    }

    bool init(stream_info_t& stream_info
              , const std::string& options)
    {
        auto* codec = utils::get_codec(stream_info.codec_info
                                       , is_encoder);

        if (codec != nullptr)
        {            
            av_context = avcodec_alloc_context3(codec);

            if (av_context != nullptr)
            {
                if (stream_info.extra_data != nullptr)

                {
                    av_context->extradata = stream_info.extra_data->data();
                    av_context->extradata_size = stream_info.extra_data->size();
                }

                if (av_context->codec_type == AVMEDIA_TYPE_AUDIO)
                {
                    stream_info.media_info >> *(av_context);

                    if (av_context->sample_fmt == sample_format_none
                            && codec->sample_fmts != nullptr)
                    {
                        av_context->sample_fmt = codec->sample_fmts[0];
                        stream_info.media_info.audio_info.sample_format = av_context->sample_fmt;
                    }

                    LOG_I << "Transcoder #" << context_id << ". Initialize audio context [" <<  av_context->sample_rate
                          << "/16/" << av_context->channels << "]" LOG_END;
                }
                else
                {
                    stream_info.media_info >> *(av_context);

                    switch(av_context->codec_id)
                    {
                        case AV_CODEC_ID_MPEG2VIDEO:
                            av_context->max_b_frames = 2;
                        break;
                        case AV_CODEC_ID_MPEG1VIDEO:
                            av_context->mb_decision = FF_MB_DECISION_RD;
                        break;
                        default:

                        break;
                    }


                    if (av_context->pix_fmt == pixel_format_none
                            && codec->pix_fmts != nullptr)
                    {
                        av_context->pix_fmt = codec->pix_fmts[0];
                        stream_info.media_info.video_info.pixel_format = av_context->pix_fmt;
                    }

                    LOG_I << "Transcoder #" << context_id << ". Initialize video context [" <<  stream_info.media_info.video_info.size.width
                          << "x" << stream_info.media_info.video_info.size.height << "@" << stream_info.media_info.video_info.fps
                          << ":" << av_get_pix_fmt_name(av_context->pix_fmt) << "]" LOG_END;
                }

                utils::merge_codec_params(*av_context
                                           , stream_info.codec_info.codec_params);


                av_init_packet(&av_packet);

                AVDictionary *av_options = nullptr;

                utils::set_extended_options(*av_context
                                            , &av_options
                                            , options);

                if (is_encoder && av_context->codec_id == AV_CODEC_ID_H264)
                {
                    // av_dict_set(&av_options, "x264opts", "bframes=0", 0);
                    av_dict_set(&av_options, "bsf", "dump_extra=freq=keyframe", 0);
                }

                av_context->workaround_bugs |= FF_BUG_AUTODETECT;

                if (!is_encoder)
                {
                    av_context->err_recognition |= AV_EF_AGGRESSIVE;
                    av_context->error_concealment |= FF_EC_GUESS_MVS | FF_EC_DEBLOCK;
                    av_context->flags |= AV_CODEC_FLAG_UNALIGNED;
                    av_context->flags |= AV_CODEC_FLAG_TRUNCATED;
                }


                auto result = avcodec_open2(av_context
                                            , av_context->codec
                                            , &av_options);

                if (av_options)
                {
                    av_dict_free(&av_options);
                }

                if (result >= 0)
                {
                    utils::update_context_info(*av_context
                                               , stream_info
                                               , av_frame);

                    LOG_I << "Transcoder #" << context_id << ". Codec " << stream_info.codec_info.to_string() << " initialized success" LOG_END;
                }
                else
                {
                    LOG_E << "Transcoder #" << context_id << ". Codec " << stream_info.codec_info.to_string()
                          << " initialized failed, error = " << result
                          << ": " <<  error_to_string(result) LOG_END;
                }

                return result >= 0;
            }

        }
        else
        {
            LOG_E << "Transcoder #" << context_id << ". Context allocate error" LOG_END;
        }

        return false;
    }

    media_data_t get_audio_data()
    {
        media_data_t audio_data;

        bool is_planar_format = av_frame.format >= AV_SAMPLE_FMT_U8P
                && av_frame.nb_samples > 1;

        auto total_samples = av_frame.nb_samples * av_frame.channels;

        audio_data.resize(total_samples * 2, 0);

        if (is_planar_format)
        {
            for (int c = 0; c < av_frame.channels && av_frame.data[c] != nullptr; c++)
            {
                auto data_ptr = audio_data.data() + c * 2;
                switch(av_frame.format)
                {
                    case AV_SAMPLE_FMT_U8P:
                        utils::sample_convert<std::int8_t>(av_frame.data[c], data_ptr, av_frame.nb_samples, av_frame.channels);
                    break;
                    case AV_SAMPLE_FMT_S16P:
                        utils::sample_convert<std::int16_t>(av_frame.data[c], data_ptr, av_frame.nb_samples, av_frame.channels);
                    break;
                    case AV_SAMPLE_FMT_S32P:
                        utils::sample_convert<std::int32_t>(av_frame.data[c], data_ptr, av_frame.nb_samples, av_frame.channels);
                    break;
                    case AV_SAMPLE_FMT_FLTP:
                        utils::sample_convert<float>(av_frame.data[c], data_ptr, av_frame.nb_samples, av_frame.channels);
                    break;
                    case AV_SAMPLE_FMT_DBLP:
                        utils::sample_convert<double>(av_frame.data[c], data_ptr, av_frame.nb_samples, av_frame.channels);
                    break;
                }
            }
        }
        else
        {
            switch(av_frame.format)
            {
                case AV_SAMPLE_FMT_U8:
                    utils::sample_convert<std::int8_t>(av_frame.data[0], audio_data.data(), total_samples);
                break;
                case AV_SAMPLE_FMT_S16:
                    utils::sample_convert<std::int16_t>(av_frame.data[0], audio_data.data(), total_samples);
                break;
                case AV_SAMPLE_FMT_S32:
                    utils::sample_convert<std::int8_t>(av_frame.data[0], audio_data.data(), total_samples);
                break;
                case AV_SAMPLE_FMT_FLT:
                    utils::sample_convert<float>(av_frame.data[0], audio_data.data(), total_samples);
                break;
                case AV_SAMPLE_FMT_DBL:
                    utils::sample_convert<double>(av_frame.data[0], audio_data.data(), total_samples);
                break;
            }
        }

        LOG_T << "Transcoder #" << context_id << ". Fetch PCM16 audio frame with size " << audio_data.size() << " bytes" LOG_END;

        av_packet.pts += av_frame.nb_samples;

        return audio_data;
    }

    bool set_audio_data(const void *data
                        , std::size_t size)
    {
        av_frame.nb_samples = size / (2 * av_frame.channels);

        if (av_frame.format == AV_SAMPLE_FMT_S16)
        {
            av_frame.data[0] = const_cast<std::uint8_t*>(static_cast<const std::uint8_t*>(data));
            av_frame.linesize[0] = size;
        }
        else
        {
            resample_buffer.resize(audio_info_t::sample_size(av_frame.format, av_frame.channels) * av_frame.nb_samples);
            auto total_samples = av_frame.nb_samples * av_frame.channels;

            bool is_planar_format = av_frame.format >= AV_SAMPLE_FMT_U8P;
            av_frame.linesize[0] = 0;

            if (is_planar_format)
            {
                auto plane_size = resample_buffer.size() /  av_frame.channels;
                for (int c = 0; c < av_frame.channels; c++)
                {
                    auto offset = plane_size * c;
                    auto data_ptr = static_cast<const std::uint8_t*>(data) + c * 2;
                    switch(av_frame.format)
                    {
                        case AV_SAMPLE_FMT_U8P:
                            utils::sample_convert<std::int16_t, std::int8_t>(data_ptr, resample_buffer.data() + offset, av_frame.nb_samples, -av_frame.channels);
                        break;
                        case AV_SAMPLE_FMT_S16P:
                            utils::sample_convert<std::int16_t, std::int16_t>(data_ptr, resample_buffer.data() + offset, av_frame.nb_samples, -av_frame.channels);
                        break;
                        case AV_SAMPLE_FMT_S32P:
                            utils::sample_convert<std::int16_t, std::int32_t>(data_ptr, resample_buffer.data() + offset, av_frame.nb_samples, -av_frame.channels);
                        break;
                        case AV_SAMPLE_FMT_FLTP:
                            utils::sample_convert<std::int16_t, float>(data_ptr, resample_buffer.data() + offset, av_frame.nb_samples, -av_frame.channels);
                        break;
                        case AV_SAMPLE_FMT_DBLP:
                            utils::sample_convert<std::int16_t, double>(data_ptr, resample_buffer.data() + offset, av_frame.nb_samples, -av_frame.channels);
                        break;
                    }
                    av_frame.data[c] = resample_buffer.data() + offset;
                }
                av_frame.linesize[0] = resample_buffer.size();
            }
            else
            {
                switch(av_frame.format)
                {
                    case AV_SAMPLE_FMT_U8:
                        utils::sample_convert<std::int16_t, std::int8_t>(av_frame.data[0], resample_buffer.data(), total_samples);
                    break;
                    case AV_SAMPLE_FMT_S32:
                        utils::sample_convert<std::int16_t, std::int32_t>(av_frame.data[0], resample_buffer.data(), total_samples);
                    break;
                    case AV_SAMPLE_FMT_FLT:
                        utils::sample_convert<std::int16_t, float>(av_frame.data[0], resample_buffer.data(), total_samples);
                    break;
                    case AV_SAMPLE_FMT_DBL:
                        utils::sample_convert<std::int16_t, double>(av_frame.data[0], resample_buffer.data(), total_samples);
                    break;
                }

                av_frame.data[0] = resample_buffer.data();
                av_frame.linesize[0] = resample_buffer.size();
            }

        }

        av_frame.pts += av_frame.nb_samples;

        LOG_T << "Transcoder #" << context_id << ". Put PCM16 audio frame with size " << av_frame.linesize[0] << " bytes" LOG_END;
        return av_frame.nb_samples > 0;
    }

    media_data_t get_video_data(std::int32_t align = default_frame_align)
    {
        std::size_t frame_size = av_image_get_buffer_size(static_cast<AVPixelFormat>(av_frame.format)
                                                          , av_frame.width
                                                          , av_frame.height
                                                          , align);

        media_data_t video_data(frame_size);

        if (frame_size > 0)
        {
            av_image_copy_to_buffer(video_data.data()
                                    , video_data.size()
                                    , av_frame.data
                                    , av_frame.linesize
                                    , static_cast<AVPixelFormat>(av_frame.format)
                                    , av_frame.width
                                    , av_frame.height
                                    , align
                                    );
        }

        if (av_context->codec_id == AV_CODEC_ID_MJPEG
                && av_frame.format == AV_PIX_FMT_YUVJ420P)
        {
            av_frame.format = AV_PIX_FMT_YUV420P;
        }

        LOG_T << "Transcoder #" << context_id << ". Fetch video frame with size " << video_data.size() << " bytes" LOG_END;

        av_packet.pts ++;

        return video_data;
    }

    bool set_video_data(const void* data
                                 , std::size_t size
                                 , std::int32_t align = default_frame_align)
    {
        std::size_t frame_size = av_image_fill_arrays(av_frame.data
                                                     , av_frame.linesize
                                                     , static_cast<const std::uint8_t*>(data)
                                                     , static_cast<AVPixelFormat>(av_frame.format)
                                                     , av_frame.width
                                                     , av_frame.height
                                                     , align);
        if (frame_size > size)
        {
            frame_size = 0;
            //LOG_T << "Transcoder #" << transcoder_id << ". Fetch video frame with size " << frame_size << " bytes" LOG_END;
        }

        LOG_T << "Transcoder #" << context_id << ". Put video frame with size " << frame_size << " bytes" LOG_END;

        av_frame.pts ++;

        return frame_size > 0;
    }

    media_data_t get_media_data()
    {
        media_data_t media_data;

        switch (av_context->codec_type)
        {
            case AVMEDIA_TYPE_AUDIO:
                media_data = std::move(get_audio_data());
            break;
            case AVMEDIA_TYPE_VIDEO:
                media_data = std::move(get_video_data());
            break;
        }

        return media_data;
    }

    bool set_media_data(const void* data
                        , std::size_t size)
    {
        switch (av_context->codec_type)
        {
            case AVMEDIA_TYPE_AUDIO:
                return set_audio_data(data
                                      , size);
            break;
            case AVMEDIA_TYPE_VIDEO:
                return set_video_data(data
                                      , size);
            break;
            default:
                av_frame.data[0] = const_cast<std::uint8_t*>(static_cast<const std::uint8_t*>(data));
                av_frame.linesize[0] = size;
        }

        return true;
    }

    bool fill_frame_info(frame_t& frame
                         , bool is_encoder)
    {
        if (is_encoder
                ? av_packet.size > 0
                : av_frame.pkt_size > 0)
        {
            frame.info.pts = av_frame.pkt_pts;
            frame.info.dts = av_frame.pkt_dts;
            frame.info.id = frame_counter;

            if (av_context->codec->type == AVMEDIA_TYPE_AUDIO)
            {
                frame.info.media_info.media_type = media_type_t::audio;
                frame.info.media_info.audio_info.sample_rate = av_frame.sample_rate;
                frame.info.media_info.audio_info.channels = av_frame.channels;
                frame.info.media_info.audio_info.sample_format = AV_SAMPLE_FMT_S16;//av_frame.format;
            }
            else
            {
                frame.info.media_info.media_type = media_type_t::video;
                frame.info.media_info.video_info.size = { av_frame.width, av_frame.height };
                frame.info.media_info.video_info.fps = av_q2d(av_context->framerate) + 0.5;
                if (!is_encoder
                        && av_context->codec_id == AV_CODEC_ID_MJPEG
                        && av_frame.format == AV_PIX_FMT_YUVJ420P)
                {
                    av_frame.format = AV_PIX_FMT_YUV420P;
                }
                frame.info.media_info.video_info.pixel_format = av_frame.format;

            }

            if (is_encoder)
            {
                frame.info.pts = av_packet.pts;
                frame.info.dts = av_packet.dts;
                frame.info.codec_id = av_context->codec_id;
                frame.info.key_frame = (av_packet.flags & AV_PKT_FLAG_KEY) != 0;
                frame.media_data = media_data_t(av_packet.data
                                                , av_packet.data + av_packet.size);

            }
            else
            {
                frame.info.pts = av_frame.pkt_pts;
                frame.info.dts = av_frame.pkt_dts;
                frame.info.codec_id = codec_id_none;
                frame.info.key_frame = av_frame.key_frame;
                frame.media_data = get_media_data();

            }

            return !frame.media_data.empty();
        }

        return false;

    }

    format_set_t get_supported_formats() const
    {
        format_set_t result;
        if (is_init)
        {
            switch (av_context->codec_type)
            {
                case AVMEDIA_TYPE_AUDIO:
                {
                    if (auto spml_fmt = av_context->codec->sample_fmts)
                    {
                        while (*spml_fmt != AV_SAMPLE_FMT_NONE)
                        {
                            result.emplace(static_cast<format_id_t>(*spml_fmt));
                            spml_fmt++;
                        }
                    }
                }
                break;
                case AVMEDIA_TYPE_VIDEO:
                {
                    if (auto pix_fmt = av_context->codec->pix_fmts)
                    {
                        while (*pix_fmt != AV_PIX_FMT_NONE)
                        {
                            result.emplace(static_cast<format_id_t>(*pix_fmt));
                            pix_fmt++;
                        }
                    }
                }
                break;
                default:;
            }
        }

        return result;
    }

    bool decode(const void* data
                , std::size_t size
                , frame_queue_t& decoded_frames
                , bool is_key_frame
                , std::int64_t timestamp)
    {

        av_packet = {};
        av_packet.data = const_cast<std::uint8_t*>(static_cast<const std::uint8_t*>(data));
        av_packet.size = size;
        av_packet.pts = AV_NOPTS_VALUE;
        av_packet.dts = AV_NOPTS_VALUE;

        if (is_key_frame)
        {
            av_packet.flags |= AV_PKT_FLAG_KEY;
        }

        if (timestamp >= 0)
        {
            av_packet.pts = timestamp;
        }

        auto result = avcodec_send_packet(av_context, &av_packet);
        bool is_fetch_picture = false;

        if (result >= 0)
        {
            while (result >= 0)
            {
                result = avcodec_receive_frame(av_context, &av_frame);

                if (result >= 0)
                {
                    frame_t decoded_frame;

                    if (fill_frame_info(decoded_frame
                                        , false))
                    {
                        frame_counter++;
                        decoded_frames.push(std::move(decoded_frame));
                        is_fetch_picture = true;
                    }
                    else
                    {
                        LOG_W << "Transcoder #" << context_id << " decode null size frame" LOG_END;
                    }
                }
                else if (result != AVERROR(EAGAIN)
                         && result != AVERROR_EOF)
                {
                    LOG_E << "Transcoder #" << context_id << ". Error call avcodec_receive_frame, err = " << result LOG_END;
                }
                else
                {                    
                    return is_fetch_picture;
                }
            }
        }
        else
        {
            LOG_E << "Transcoder #" << context_id << ". Error avcodec_send_packet, err = " << result LOG_END;
        }

        return false;
    }

    bool encode(const void* data
                , std::size_t size
                , frame_queue_t& encoded_frames
                , bool is_key_frame
                , std::int64_t timestamp)
    {
        std::int32_t result = -1;

        if (set_media_data(data
                           , size))
        {
            av_frame.key_frame = static_cast<std::int32_t>(is_key_frame);

            av_frame.pict_type = is_key_frame
                    ? AV_PICTURE_TYPE_I
                    : AV_PICTURE_TYPE_NONE;

            av_frame.extended_data = av_frame.data;

            if (timestamp != 0)
            {
                av_frame.pkt_dts = AV_NOPTS_VALUE;
                av_frame.pkt_pts = timestamp;
            }

            result = avcodec_send_frame(av_context, &av_frame);

            bool is_push_picture = false;

            if (result >= 0)
            {
                while (result >= 0)
                {
                    result = avcodec_receive_packet(av_context, &av_packet);

                    if (result >= 0)
                    {
                        frame_t encoded_frame;

                        if (fill_frame_info(encoded_frame
                                            , true))
                        {
                            is_push_picture = true;
                            frame_counter++;

                            /*
                            if (av_context->codec_type == AVMEDIA_TYPE_VIDEO)
                            {

                                std::cout << "Encoder Video frame #" << frame_counter
                                          << ", size: " << encoded_frame.media_data.size()
                                          << ", data: " << base::hex_dump(encoded_frame.media_data.data()
                                                                          , encoded_frame.media_data.size())
                                          << std::endl;
                            }*/

                            encoded_frames.push(std::move(encoded_frame));
                        }
                        else
                        {
                            LOG_W << "Transcoder #" << context_id << " encode null size frame" LOG_END;
                        }
                    }
                    else if (result != AVERROR(EAGAIN)
                              && result != AVERROR_EOF)
                    {
                        LOG_E << "Transcoder #" << context_id << ". Error call avcodec_receive_frame, err = " << result LOG_END;
                    }
                    else
                    {
                        return is_push_picture;
                    }
                }

            }
            else
            {
                LOG_E << "Transcoder #" << context_id << ". Error avcodec_send_packet, err = " << result LOG_END;
            }

        }

        return false;
    }
};

struct libav_transcoder_context_t
{
    using u_ptr_t = std::unique_ptr<libav_transcoder_context_t>;

    frame_handler_t                         m_frame_handler;
    libav_codec_context_t::u_ptr_t          m_codec_context;
    stream_info_t                           m_stream_info;
    transcoder_type_t                       m_transcoder_type;

    static u_ptr_t create(const frame_handler_t& frame_handler)
    {
        return std::make_unique<libav_transcoder_context_t>(frame_handler);
    }

    libav_transcoder_context_t(const frame_handler_t& frame_handler)
        : m_frame_handler(frame_handler)
        , m_codec_context(nullptr)
        , m_transcoder_type(transcoder_type_t::unknown)

    {

    }

    ~libav_transcoder_context_t()
    {
        close();
    }

    bool open(const stream_info_t& steam_info
              , transcoder_type_t transcoder_type
              , const std::string& options)
    {
        close();
        if (transcoder_type != transcoder_type_t::unknown)
        {
            m_stream_info = steam_info;
            m_transcoder_type = transcoder_type;
            m_codec_context.reset(new libav_codec_context_t(m_stream_info
                                                            , m_transcoder_type == transcoder_type_t::encoder
                                                            , options));

            if (!m_codec_context->is_init)
            {
                close();
            }
        }

        return m_codec_context != nullptr;
    }

    bool close()
    {
        if (m_codec_context != nullptr)
        {
            m_codec_context.reset();
            m_transcoder_type = transcoder_type_t::unknown;
            m_stream_info = {};

            return true;
        }

        return false;
    }

    bool is_open() const
    {
        return m_codec_context != nullptr;
    }

    format_set_t supported_formats() const
    {
        return m_codec_context != nullptr
                ? m_codec_context->get_supported_formats()
                : format_set_t{};
    }

    bool transcode(const void* data
                   , std::size_t size
                   , frame_queue_t& frame_queue
                   , transcode_flag_t transcode_flags
                   , std::int64_t timestamp)
    {
        if (m_codec_context != nullptr)
        {
            switch(m_transcoder_type)
            {
                case transcoder_type_t::encoder:
                    return m_codec_context->encode(data
                                                   , size
                                                   , frame_queue
                                                   , transcode_flags & transcode_flag_t::key_frame
                                                   , timestamp);
                break;
                case transcoder_type_t::decoder:
                    return m_codec_context->decode(data
                                                   , size
                                                   , frame_queue
                                                   , transcode_flags & transcode_flag_t::key_frame
                                                   , timestamp);
                break;
            }
        }

        return false;
    }
};
//------------------------------------------------------------------------------
libav_transcoder::u_ptr_t libav_transcoder::create(const frame_handler_t& frame_handler)
{
    return std::make_unique<libav_transcoder>(frame_handler);
}


libav_transcoder::libav_transcoder(const frame_handler_t& frame_handler)
    : m_transcoder_context(libav_transcoder_context_t::create(frame_handler))
{
    LOG_T << "Create libav transcoder" LOG_END;
}

libav_transcoder::~libav_transcoder()
{

}

bool libav_transcoder::open(const stream_info_t &steam_info
                            , transcoder_type_t transcoder_type
                            , const std::string& options)
{
    LOG_D << "Open transcoder by codec " << steam_info.codec_info.to_string() LOG_END;
    return m_transcoder_context->open(steam_info
                                      , transcoder_type
                                      , options);
}

bool libav_transcoder::close()
{
    LOG_D << "Close transcoder" LOG_END;
    return m_transcoder_context->close();
}

bool libav_transcoder::is_open() const
{
    return m_transcoder_context->is_open();
}

format_set_t libav_transcoder::supported_formats() const
{
    return m_transcoder_context->supported_formats();
}

transcoder_type_t libav_transcoder::type() const
{
    return m_transcoder_context->m_transcoder_type;
}

const stream_info_t &libav_transcoder::config() const
{
    return m_transcoder_context->m_stream_info;
}

frame_queue_t libav_transcoder::transcode(const void *data
                                          , std::size_t size
                                          , transcode_flag_t transcode_flags
                                          , std::int64_t timestamp)
{
    LOG_D << "Transcode frame size = " << size LOG_END;

    frame_queue_t frame_queue;

    m_transcoder_context->transcode(data
                                    , size
                                    , frame_queue
                                    , transcode_flags
                                    , timestamp);

    return frame_queue;
}

bool libav_transcoder::transcode(const void *data
                                 , std::size_t size
                                 , frame_queue_t &frame_queue
                                 , transcode_flag_t transcode_flags
                                 , std::int64_t timestamp)
{
    LOG_D << "Transcode frame size = " << size LOG_END;

    return m_transcoder_context->transcode(data
                                           , size
                                           , frame_queue
                                           , transcode_flags
                                           , timestamp);
}

}
