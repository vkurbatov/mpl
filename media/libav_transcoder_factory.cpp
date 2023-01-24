#include "libav_transcoder_factory.h"

#include "core/convert_utils.h"
#include "core/option_helper.h"

#include "core/smart_buffer.h"
#include "core/i_buffer_collection.h"

#include "core/message_event_impl.h"
#include "core/event_channel_state.h"

#include "core/data_splitter.h"

#include "media_option_types.h"
#include "message_frame_impl.h"
#include "audio_frame_impl.h"
#include "video_frame_impl.h"

#include "audio_format_helper.h"

#include "tools/ffmpeg/libav_transcoder.h"

namespace mpl::media
{

namespace detail
{

i_media_format::u_ptr_t clone_format(const i_media_format& media_format)
{
    switch(media_format.media_type())
    {
        case media_type_t::audio:
            return audio_format_impl::create(static_cast<const i_audio_format&>(media_format));
        break;
        case media_type_t::video:
            return video_format_impl::create(static_cast<const i_video_format&>(media_format));
        break;
        default:;
    }

    return nullptr;
}

bool compare_format_id(const i_media_format& lhs
                       , const i_media_format& rhs)
{
    if (lhs.media_type() == rhs.media_type())
    {
        switch(lhs.media_type())
        {
            case media_type_t::audio:
                return static_cast<const i_audio_format&>(lhs).format_id()
                        == static_cast<const i_audio_format&>(rhs).format_id();
            break;
            case media_type_t::video:
                return static_cast<const i_video_format&>(lhs).format_id()
                        == static_cast<const i_video_format&>(rhs).format_id();
            break;
            default:;
        }
    }

    return false;
}

bool is_key_frame(const i_media_frame& frame)
{
    if (frame.media_type() == media_type_t::video)
    {
        auto frame_type = static_cast<const i_video_frame&>(frame).frame_type();
        return frame_type == i_video_frame::frame_type_t::key_frame
                && frame_type == i_video_frame::frame_type_t::image_frame;
    }
    return false;
}

}

class libav_transcoder : public i_media_converter
{
    ffmpeg::libav_transcoder    m_native_transcoder;
    i_message_sink*             m_output_sink;

    i_media_format::u_ptr_t     m_input_format;
    i_media_format::u_ptr_t     m_output_format;

    data_splitter               m_frame_splitter;

    frame_id_t                  m_frame_id;
    bool                        m_wait_first_frame;

    bool                        m_is_init;

public:
    using u_ptr_t = std::unique_ptr<libav_transcoder>;

    static u_ptr_t create(const i_media_format &media_format
                          , bool encoder)
    {
        if (auto transcoder = std::make_unique<libav_transcoder>(media_format
                                                                 , encoder))
        {
            if (transcoder->is_init())
            {
                return transcoder;
            }
        }

        return nullptr;
    }

    libav_transcoder(const i_media_format &media_format
                     , bool encoder)
        : m_output_sink(nullptr)
        , m_frame_splitter(0)
        , m_frame_id(0)
        , m_wait_first_frame(media_format.media_type() == media_type_t::video)

    {
        m_is_init = initialize(media_format
                               , encoder);
    }

    ~libav_transcoder()
    {
        m_native_transcoder.close();
    }

    bool check_format(const i_media_format& format)
    {
        if (m_input_format->is_compatible(format))
        {
            return true;
        }

        if (m_input_format->media_type() == format.media_type())
        {
            switch(m_native_transcoder.type())
            {
                case ffmpeg::transcoder_type_t::decoder:
                    return detail::compare_format_id(*m_input_format
                                                     , format);
                break;
                default:;
            }
        }

        return false;
    }

    bool check_frame(const i_media_frame& frame)
    {
        switch(frame.media_type())
        {
            case media_type_t::audio:
                return check_format(static_cast<const i_audio_frame&>(frame).format());
            break;
            case media_type_t::video:
                return check_format(static_cast<const i_video_frame&>(frame).format());
            break;
            default:;

        }

        return false;
    }

    bool initialize(const i_media_format &media_format
                    , bool encoder)
    {
        if (media_format.is_encoded())
        {
            switch(media_format.media_type())
            {
                case media_type_t::audio:
                case media_type_t::video:
                {
                    ffmpeg::stream_info_t stream_info;
                    if (core::utils::convert(media_format
                                             , stream_info))
                    {
                        option_reader options(media_format.options());
                        std::string transcoder_options = options.get<std::string>(opt_codec_params, {});
                        if (m_native_transcoder.open(stream_info
                                                     , encoder
                                                     ? ffmpeg::transcoder_type_t::encoder
                                                     : ffmpeg::transcoder_type_t::decoder
                                                     , transcoder_options))
                        {
                            m_input_format = detail::clone_format(media_format);
                            m_output_format = detail::clone_format(media_format);

                            auto& tune_format = encoder
                                                ? *m_input_format
                                                : *m_output_format;

                            auto tune_stream_info = m_native_transcoder.config();
                            tune_stream_info.codec_info.id = ffmpeg::codec_id_none;

                            switch(media_format.media_type())
                            {
                                case media_type_t::audio:
                                {
                                    audio_format_impl& audio_format = static_cast<audio_format_impl&>(tune_format);
                                    if (core::utils::convert(stream_info
                                                             , audio_format))
                                    {
                                        audio_format.set_format_id(audio_format_id_t::pcm16);
                                        if (encoder
                                            && tune_stream_info.codec_info.codec_params.frame_size > 0)
                                        {
                                            m_frame_splitter.reset(tune_stream_info.codec_info.codec_params.frame_size
                                                                   * audio_format_helper(audio_format).sample_size());
                                        }

                                        return true;
                                    }
                                }
                                break;
                                case media_type_t::video:
                                {
                                    video_format_impl& video_format = static_cast<video_format_impl&>(tune_format);
                                    if (core::utils::convert(stream_info
                                                             , video_format))
                                    {
                                        return true;
                                    }
                                }
                                break;
                                default:;
                            }

                        }
                    }
                }
                break;
                default:;
            }

        }

        return false;
    }


    bool push_frame(ffmpeg::frame_t&& libav_frame
                   , const i_media_frame& input_frame
                   , timestamp_t timestamp)
    {
        if (m_output_sink)
        {

            switch(input_frame.media_type())
            {
                case media_type_t::audio:
                {
                    audio_frame_impl audio_frame(static_cast<const i_audio_format&>(*m_output_format)
                                                 , m_frame_id++
                                                 , timestamp);

                    audio_frame.audio_format().set_sample_rate(libav_frame.info.media_info.audio_info.sample_rate);
                    audio_frame.audio_format().set_sample_rate(libav_frame.info.media_info.audio_info.channels);

                    audio_frame.smart_buffers().set_buffer(main_media_buffer_index
                                                           , smart_buffer(std::move(libav_frame.media_data)));

                    return m_output_sink->send_message(message_frame_ref_impl(audio_frame));
                }
                break;
                case media_type_t::video:
                {
                    video_frame_impl video_frame(static_cast<const i_video_format&>(*m_output_format)
                                                 , m_frame_id++
                                                 , timestamp);

                    video_frame.video_format().set_width(libav_frame.info.media_info.video_info.size.width);
                    video_frame.video_format().set_height(libav_frame.info.media_info.video_info.size.height);
                    video_frame.video_format().set_frame_rate(libav_frame.info.media_info.video_info.fps);

                    video_frame.smart_buffers().set_buffer(main_media_buffer_index
                                                           , smart_buffer(std::move(libav_frame.media_data)));

                    return m_output_sink->send_message(message_frame_ref_impl(video_frame));
                }
                break;
                default:;
            }
        }

        return false;
    }

    bool on_media_frame(const i_media_frame& media_frame)
    {
        if (check_frame(media_frame))
        {
            if (auto buffer = media_frame.buffers().get_buffer(main_media_buffer_index))
            {
                ffmpeg::frame_queue_t frame_queue;
                auto frame_time = media_frame.timestamp();
                std::int32_t result = 0;

                auto key_frame = detail::is_key_frame(media_frame);

                auto flag = key_frame
                        ? ffmpeg::transcode_flag_t::key_frame
                        : ffmpeg::transcode_flag_t::none;

                if (m_wait_first_frame)
                {
                    if (!key_frame)
                    {
                        return false;
                    }

                    m_wait_first_frame = false;
                }

                if (m_frame_splitter.fragment_size() > 0
                        && media_frame.media_type() == media_type_t::audio)
                {
                    auto sample_size = audio_format_helper(static_cast<const i_audio_frame&>(media_frame).format()).sample_size();
                    auto samples = m_frame_splitter.buffered_size() / sample_size;

                    auto queue = m_frame_splitter.push_stream(buffer->data()
                                                              , buffer->size());

                    frame_time -= samples;

                    while(!queue.empty() && result >= 0)
                    {

                        if (m_native_transcoder.transcode(queue.front().data()
                                                              , queue.front().size()
                                                              , frame_queue
                                                              , flag))
                        {
                            result++;
                        }

                        queue.pop();
                    }
                }
                else
                {
                    result = m_native_transcoder.transcode(buffer->data()
                                                           , buffer->size()
                                                           , frame_queue
                                                           , flag
                                                           , frame_time);
                }

                if (result > 0)
                {
                    while(!frame_queue.empty())
                    {
                        push_frame(std::move(frame_queue.front())
                                             , media_frame
                                             , frame_time);

                        frame_queue.pop();
                    }
                }

            }
        }
        return false;
    }

    const i_media_format& transcoder_format() const
    {
        return m_native_transcoder.type() == ffmpeg::transcoder_type_t::encoder
                ? *m_output_format
                : *m_input_format;
    }

    media_type_t media_type() const
    {
        return transcoder_format().media_type();
    }

    bool is_init() const
    {
        return m_is_init;
    }

    // i_message_sink interface
public:
    bool send_message(const i_message &message) override
    {
        if (message.category() == message_category_t::frame)
        {
            return on_media_frame(static_cast<const i_message_frame&>(message).frame());
        }

        return false;
    }

    // i_media_converter interface
public:
    const i_media_format &input_format() const override
    {
        return *m_input_format;
    }
    const i_media_format &output_format() const override
    {
        return *m_output_format;
    }

    void set_sink(i_message_sink *output_sink) override
    {
        m_output_sink = output_sink;
    }
};

libav_transcoder_factory::libav_transcoder_factory(bool encoder_factory)
    : m_encoder_factory(encoder_factory)
{


}

i_media_converter::u_ptr_t libav_transcoder_factory::create_converter(const i_media_format &media_format)
{
    return libav_transcoder::create(media_format
                                    , m_encoder_factory);
}


}
