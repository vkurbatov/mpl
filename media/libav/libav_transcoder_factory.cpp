#include "libav_transcoder_factory.h"

#include "utils/property_reader.h"
#include "utils/convert_utils.h"
#include "utils/option_helper.h"

#include "utils/smart_buffer.h"
#include "core/i_buffer_collection.h"

#include "utils/message_event_impl.h"
#include "core/event_channel_state.h"

//#include "core/data_splitter.h"
#include "media/audio_frame_splitter.h"

#include "media/media_option_types.h"
#include "media/audio_frame_impl.h"
#include "media/video_frame_impl.h"
#include "media/video_format_info.h"

#include "media/audio_format_helper.h"

#include "tools/ffmpeg/libav_transcoder.h"

namespace mpl::media
{

namespace detail
{

template<media_type_t MediaType>
struct format_types_t;

template<>
struct format_types_t<media_type_t::audio>
{
    using i_format_t = i_audio_format;
    using i_frame_t = i_audio_frame;
    using format_impl_t = audio_format_impl;
    using frame_impl_t = audio_frame_impl;
};

template<>
struct format_types_t<media_type_t::video>
{
    using i_format_t = i_video_format;
    using i_frame_t = i_video_frame;
    using format_impl_t = video_format_impl;
    using frame_impl_t = video_frame_impl;
};


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
                && video_format_info_t::get_info(static_cast<const i_video_frame&>(frame).format().format_id()).motion;
    }
    return false;
}

template<typename FrameImpl>
void set_key_frame(FrameImpl& frame_impl, bool key_frame)
{
    // nothing
}

template<>
void set_key_frame(video_frame_impl& frame_impl, bool key_frame)
{
    frame_impl.set_frame_type(key_frame
                              ? i_video_frame::frame_type_t::key_frame
                              : i_video_frame::frame_type_t::delta_frame);
}

template<typename FormatImpl>
void tune_frame_format(const pt::ffmpeg::frame_t& libav_frame
                 , FormatImpl& format);


template<>
void tune_frame_format(const pt::ffmpeg::frame_t& libav_frame
                 , audio_format_impl& format)
{
    audio_format_id_t format_id = format.format_id();
    if (utils::convert(libav_frame.format_info()
                       , format_id))
    {
        format.set_format_id(format_id);
    }

    format.set_sample_rate(libav_frame.info.media_info.audio_info.sample_rate);
    format.set_channels(libav_frame.info.media_info.audio_info.channels);
}

template<>
void tune_frame_format(const pt::ffmpeg::frame_t& libav_frame
                      , video_format_impl& format)
{
    video_format_id_t format_id = format.format_id();
    if (utils::convert(libav_frame.format_info()
                       , format_id))
    {
        format.set_format_id(format_id);
    }

    format.set_width(libav_frame.info.media_info.video_info.size.width);
    format.set_height(libav_frame.info.media_info.video_info.size.height);
    format.set_frame_rate(libav_frame.info.media_info.video_info.fps);
}

std::uint32_t get_sample_size(const i_media_format& format)
{
    switch(format.media_type())
    {
        case media_type_t::audio:
            return audio_format_helper(static_cast<const i_audio_format&>(format)).sample_size();
        break;
        default:;
    }

    return 0;
}

inline bool is_wait_key_frame(const i_media_format& media_format)
{
    return media_format.media_type() == media_type_t::video
            && video_format_info_t::get_info(static_cast<const i_video_format&>(media_format).format_id()).motion;
}


}

template<media_type_t MediaType>
class libav_transcoder : public i_media_converter
{
    using i_format_t = typename detail::format_types_t<MediaType>::i_format_t;
    using i_frame_t = typename detail::format_types_t<MediaType>::i_frame_t;
    using format_impl_t = typename detail::format_types_t<MediaType>::format_impl_t;
    using frame_impl_t = typename detail::format_types_t<MediaType>::frame_impl_t;

    pt::ffmpeg::libav_transcoder    m_native_transcoder;
    i_message_sink*             m_output_sink;

    format_impl_t               m_input_format;
    format_impl_t               m_output_format;

    audio_frame_splitter        m_frame_splitter;

    frame_id_t                  m_frame_id;
    bool                        m_wait_first_frame;

    bool                        m_is_init;

public:
    using u_ptr_t = std::unique_ptr<libav_transcoder>;

    static u_ptr_t create(const i_property &property
                          , bool encoder)
    {
        property_reader reader(property);
        format_impl_t media_format;
        if (reader.get("format", media_format)
                && media_format.is_encoded())
        {
            if (auto transcoder = std::make_unique<libav_transcoder>(std::move(media_format)
                                                                     , encoder))
            {
                if (transcoder->is_init())
                {
                    return transcoder;
                }
            }
        }

        return nullptr;
    }

    libav_transcoder(format_impl_t&& media_format
                     , bool encoder)
        : m_output_sink(nullptr)
        , m_output_format(std::move(media_format))
        , m_frame_id(0)
        , m_wait_first_frame(false)

    {
        m_is_init = initialize(m_output_format
                               , encoder);
    }

    ~libav_transcoder()
    {
        m_native_transcoder.close();
    }

    bool check_format(const i_format_t& format)
    {
        if (m_input_format.is_compatible(format))
        {
            return true;
        }

        if (m_input_format.media_type() == format.media_type())
        {
            switch(m_native_transcoder.type())
            {
                case pt::ffmpeg::transcoder_type_t::decoder:
                    return detail::compare_format_id(m_input_format
                                                     , format);
                break;
                default:;
            }
        }

        return false;
    }

    bool check_frame(const i_frame_t& frame)
    {
        return check_format(frame.format());
    }

    void tune_format(audio_format_impl& format)
    {
        auto tune_stream_info = m_native_transcoder.config();
        tune_stream_info.codec_info.id = pt::ffmpeg::codec_id_none;

        if (m_native_transcoder.type() == pt::ffmpeg::transcoder_type_t::encoder
            && tune_stream_info.codec_info.codec_params.frame_size > 0)
        {
            m_frame_splitter.setup(format
                                   , tune_stream_info.codec_info.codec_params.frame_size);
            /*m_frame_splitter.reset(tune_stream_info.codec_info.codec_params.frame_size
                                   * audio_format_helper(format).sample_size());*/
        }
    }

    void tune_format(video_format_impl& format)
    {
        return;
    }

    bool initialize(const i_format_t &media_format
                    , bool encoder)
    {
        if (media_format.is_encoded())
        {
            switch(media_format.media_type())
            {
                case media_type_t::audio:
                case media_type_t::video:
                {
                    pt::ffmpeg::stream_info_t stream_info;
                    if (utils::convert(media_format
                                       , stream_info))
                    {
                        option_reader options(media_format.options());
                        std::string transcoder_options = options.get<std::string>(opt_codec_params, {});
                        if (m_native_transcoder.open(stream_info
                                                     , encoder
                                                     ? pt::ffmpeg::transcoder_type_t::encoder
                                                     : pt::ffmpeg::transcoder_type_t::decoder
                                                     , transcoder_options))
                        {
                            m_input_format.assign(media_format);
                            m_output_format.assign(media_format);

                            auto& target_format = encoder
                                                ? m_input_format
                                                : m_output_format;

                            auto tune_stream_info = m_native_transcoder.config();
                            tune_stream_info.codec_info.id = pt::ffmpeg::codec_id_none;

                            if (utils::convert(tune_stream_info
                                               , target_format))
                            {
                                tune_format(target_format);
                                m_wait_first_frame = !encoder
                                        && detail::is_wait_key_frame(media_format);

                                return true;
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


    bool push_frame(pt::ffmpeg::frame_t&& libav_frame
                   , const i_frame_t& input_frame
                   , timestamp_t timestamp)
    {
        if (m_output_sink)
        {
            auto format_frame = m_output_format;
            format_frame.set_options(input_frame.format().options());
            detail::tune_frame_format(libav_frame
                                      , format_frame);

            frame_impl_t frame(format_frame
                               , m_frame_id++
                               , timestamp);

            frame.set_ntp_timestamp(input_frame.ntp_timestamp());
            frame.set_options(input_frame.options());

            detail::set_key_frame(frame
                                  , libav_frame.info.key_frame);

            frame.smart_buffers().set_buffer(media_buffer_index
                                             , smart_buffer(std::move(libav_frame.media_data)));

            // frame.set_options(input_frame.options());

            return m_output_sink->send_message(frame);
        }

        return false;
    }

    bool on_media_frame(const i_frame_t& media_frame)
    {
        if (check_frame(media_frame))
        {
            if (auto buffer = media_frame.data().get_buffer(media_buffer_index))
            {
                pt::ffmpeg::frame_queue_t frame_queue;
                auto frame_time = media_frame.timestamp();
                std::int32_t result = 0;

                auto key_frame = detail::is_key_frame(media_frame);

                auto flag = key_frame
                        ? pt::ffmpeg::transcode_flag_t::key_frame
                        : pt::ffmpeg::transcode_flag_t::none;


                if (m_wait_first_frame)
                {
                    if (!key_frame)
                    {
                        // need key frame for decoder
                        return false;
                    }

                    m_wait_first_frame = false;
                }


                if (m_frame_splitter.frame_size() > 0
                        && media_frame.media_type() == media_type_t::audio)
                {
                    auto samples = m_frame_splitter.buffered_samples();

                    auto queue = m_frame_splitter.push_frame(buffer->data()
                                                              , buffer->size());

                    frame_time -= samples;

                    while(!queue.empty()
                          && result >= 0)
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

                        // auto samples = frame_queue.front().media_data.size();
                        push_frame(std::move(frame_queue.front())
                                             , media_frame
                                             , frame_time);


                        frame_time += m_native_transcoder.config().codec_info.codec_params.frame_size;

                        frame_queue.pop();
                    }

                    return true;
                }

            }
        }
        return false;
    }

    const i_media_format& transcoder_format() const
    {
        return m_native_transcoder.type() == pt::ffmpeg::transcoder_type_t::encoder
                ? m_output_format
                : m_input_format;
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
        if (message.category() == message_category_t::data)
        {
            const auto& frame = static_cast<const i_media_frame&>(message);
            if (frame.media_type() == MediaType)
            {
                return on_media_frame(static_cast<const i_frame_t&>(frame));
            }
        }

        return false;
    }

    // i_media_converter interface
public:
    const i_media_format &input_format() const override
    {
        return m_input_format;
    }
    const i_media_format &output_format() const override
    {
        return m_output_format;
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

libav_transcoder_factory &libav_transcoder_factory::encoder_factory()
{
    static libav_transcoder_factory single_encoder_factory(true);
    return single_encoder_factory;
}

libav_transcoder_factory &libav_transcoder_factory::decoder_factory()
{
    static libav_transcoder_factory single_decoder_factory(false);
    return single_decoder_factory;
}

i_media_converter::u_ptr_t libav_transcoder_factory::create_converter(const i_property& params)
{
    property_reader reader(params);
    switch(reader.get<media_type_t>("format.media_type"
                                    , media_type_t::undefined))
    {
        case media_type_t::audio:
            return libav_transcoder<media_type_t::audio>::create(params
                                                                , m_encoder_factory);
        break;
        case media_type_t::video:
            return libav_transcoder<media_type_t::video>::create(params
                                                                 , m_encoder_factory);
        break;
        default:;
    }

    return nullptr;
}


}
