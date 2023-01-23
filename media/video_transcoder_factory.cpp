#include "video_transcoder_factory.h"
#include "video_format_impl.h"

#include "core/convert_utils.h"
#include "core/option_helper.h"

#include "i_message_frame.h"
#include "i_video_frame.h"

#include "core/message_event_impl.h"
#include "core/event_channel_state.h"

#include "core/message_sink_impl.h"
#include "core/message_router_impl.h"

#include "core/smart_buffer.h"
#include "core/i_buffer_collection.h"

#include "media_option_types.h"

#include "tools/ffmpeg/libav_converter.h"
#include "tools/ffmpeg/libav_transcoder.h"


namespace mpl::media
{


namespace detail
{
    bool fragment_info_from_format(const i_video_format& format
                                   , ffmpeg::fragment_info_t& fragment_info)
    {
        ffmpeg::stream_info_t stream_info;
        if (core::utils::convert(format.format_id()
                                 , stream_info))
        {
            fragment_info.pixel_format = stream_info.media_info.video_info.pixel_format;
            fragment_info.frame_size = stream_info.media_info.video_info.size;

            return true;
        }

        return false;
    }

}

struct ffmpeg_converter
{
    ffmpeg::libav_converter m_native_converter;

    ffmpeg::fragment_info_t m_input_fragment;
    ffmpeg::fragment_info_t m_output_fragment;

    using u_ptr_t = std::unique_ptr<ffmpeg_converter>;

    static u_ptr_t create(const i_video_format& input_format
                          , const i_video_format& output_format)
    {
        if (input_format.is_convertable()
                && output_format.is_convertable())
        {
            ffmpeg::fragment_info_t input_fragment;
            ffmpeg::fragment_info_t output_fragment;
            if (detail::fragment_info_from_format(input_format, input_fragment)
                    && detail::fragment_info_from_format(output_format, output_fragment))
            {
                return std::make_unique<ffmpeg_converter>(input_fragment
                                                          , output_fragment);
            }
        }

        return nullptr;
    }

    ffmpeg_converter(const ffmpeg::fragment_info_t& input_fragment
                     , const ffmpeg::fragment_info_t& output_fragment)
        : m_input_fragment(input_fragment)
        , m_output_fragment(output_fragment)
    {

    }

    bool convert(const void* input_data
                 , void* output_data)
    {
        return m_native_converter.convert_frames(m_input_fragment
                                                 , input_data
                                                 , m_output_fragment
                                                 , output_data) > 0;
    }
};

/*
struct ffmpeg_transcoder
{
    ffmpeg::libav_transcoder    m_transcoder;
public:
    using u_ptr_t = std::unique_ptr<ffmpeg_transcoder>;

    static u_ptr_t create(const i_video_format& format
                          , bool is_encoder)
    {
        if (format.is_encoded())
        {
            ffmpeg::stream_info_t stream_info;

            if (mpl::core::utils::convert(format, stream_info))
            {
                option_reader options(format.options());
                std::string transcoder_options = options.get<std::string>(opt_codec_params, {});

                if (auto transcoder = std::make_unique<ffmpeg_transcoder>(stream_info
                                                                          , is_encoder
                                                                          ? ffmpeg::transcoder_type_t::encoder
                                                                          : ffmpeg::transcoder_type_t::decoder
                                                                          , transcoder_options))
                {
                    if (transcoder->is_init())
                    {
                        return transcoder;
                    }
                }

            }

        }

        return nullptr;
    }

    ffmpeg_transcoder(const ffmpeg::stream_info_t& stream_info
                      , ffmpeg::transcoder_type_t transcoder_type
                      , const std::string& options)
    {
        m_transcoder.open(stream_info
                          , transcoder_type
                          , options);
    }

    ~ffmpeg_transcoder()
    {
        m_transcoder.close();
    }

    bool is_init() const
    {
        return m_transcoder.is_open();
    }

};
*/


struct complex_transcoder
{    
    using frame_handler_t = std::function<bool(const i_video_frame& video_frame)>;

    ffmpeg::libav_transcoder::u_ptr_t   m_decoder;
    ffmpeg_converter::u_ptr_t           m_converter;
    ffmpeg::libav_transcoder::u_ptr_t   m_encoder;

    video_format_impl                   m_input_format;
    video_format_impl                   m_output_format;

    frame_handler_t                     m_frame_handler;

    bool                                m_is_init;

    static ffmpeg_converter::u_ptr_t create_converter(const i_video_format& input_format
                                                      , const i_video_format& output_format)
    {
        return ffmpeg_converter::create(input_format
                                        , output_format);
    }

    static ffmpeg::libav_transcoder::u_ptr_t create_transcoder(const i_video_format& format
                                                               , bool encoder)
    {
        if (format.is_encoded())
        {
            ffmpeg::stream_info_t stream_info;

            if (mpl::core::utils::convert(format, stream_info))
            {
                if (auto transcoder = ffmpeg::libav_transcoder::create())
                {
                    option_reader options(format.options());
                    std::string transcoder_options = options.get<std::string>(opt_codec_params, {});
                    if (transcoder->open(stream_info
                                         , encoder
                                         ? ffmpeg::transcoder_type_t::encoder
                                         : ffmpeg::transcoder_type_t::decoder
                                         , transcoder_options))
                    {
                        return transcoder;
                    }
                }
            }

        }

        return nullptr;
    }

    static ffmpeg::libav_transcoder::u_ptr_t create_encoder(const i_video_format& format)
    {
        return create_transcoder(format
                                 , true);
    }

    static ffmpeg::libav_transcoder::u_ptr_t create_decoder(const i_video_format& format)
    {
        return create_transcoder(format
                                 , false);
    }

    complex_transcoder(const i_video_format& output_format
                       , const frame_handler_t& frame_handler)
        : m_output_format(output_format)
        , m_frame_handler(frame_handler)
        , m_is_init(false)
    {

    }

    void reset()
    {
        m_is_init = false;
        m_decoder.reset();
        m_converter.reset();
        m_encoder.reset();
    }

    bool reinitialize()
    {
        reset();

        if (m_input_format.is_compatible(m_output_format))
        {
            return true;
        }

        video_format_impl input_convert_format(m_input_format);
        video_format_impl output_convert_format(m_output_format);

        // encoder

        if (m_output_format.is_encoded())
        {
            if (m_encoder = create_encoder(m_output_format))
            {
                for (const auto& f : m_encoder->supported_formats())
                {
                    video_format_id_t format_id;
                    if (core::utils::convert(ffmpeg::format_info_t(f)
                                             , format_id))
                    {
                        output_convert_format.set_format_id(format_id);
                        break;
                    }
                }
            }
            else
            {
                return false;
            }
        }

        // encoder

        if (m_input_format.is_encoded())
        {
            if (m_decoder = create_decoder(m_input_format))
            {
                for (const auto& f : m_decoder->supported_formats())
                {
                    video_format_id_t format_id;
                    if (core::utils::convert(ffmpeg::format_info_t(f)
                                             , format_id))
                    {
                        input_convert_format.set_format_id(format_id);
                        break;
                    }
                }
            }
            else
            {
                return false;
            }
        }

        if (!input_convert_format.is_compatible(output_convert_format))
        {
            if (m_converter = create_converter(input_convert_format
                                               , output_convert_format))
            {

            }
            else
            {
                return false;
            }
        }

        return true;
    }

    bool is_transit_transcoder() const
    {
        return m_input_format.is_compatible(m_output_format);
    }

    bool check_or_update_input_format(const i_video_format& input_format
                                      , bool key_frame = false)
    {
        bool need_reinitialize = false;

        if (!m_is_init
                || !m_input_format.is_compatible(input_format))
        {
            if (key_frame)
            {
                m_input_format.assign(input_format);
                need_reinitialize = true;
            }
            else
            {
                return false;
            }
        }

        if (need_reinitialize)
        {
            m_is_init = reinitialize();
        }

        return m_is_init;
    }

    bool push_video_frame(const i_video_frame& frame)
    {
        bool key_frame = frame.frame_type() == i_video_frame::frame_type_t::key_frame
                || frame.frame_type() == i_video_frame::frame_type_t::image_frame;
        if (check_or_update_input_format(frame.format()
                                         , key_frame))
        {
            if (is_transit_transcoder())
            {
                return m_frame_handler(frame);
            }
            else
            {

            }
        }

        return false;
    }

    void set_output_format(const i_video_format& format)
    {
        if (!m_output_format.is_equal(format))
        {
            reset();
            m_output_format.assign(format);
        }
    }
};

class video_transcoder : public i_media_transcoder
{
    video_format_impl       m_input_format;
    video_format_impl       m_output_format;

    message_sink_impl       m_sink;
    message_router_impl     m_source_router;

public:
    using u_ptr_t = std::unique_ptr<video_transcoder>;

    static u_ptr_t create(const i_property &property)
    {
        return nullptr;
    }

    video_transcoder(video_format_impl&& input_format
                     , video_format_impl&& output_format)
        : m_input_format(std::move(input_format))
        , m_output_format(std::move(output_format))
        , m_sink([&](const auto& message) { return on_sink_message(message); })
    {

    }

    ~video_transcoder()
    {

    }

    bool on_sink_video_frame(const i_option& options
                             , const i_video_frame& video_frame)
    {
        if (m_input_format.is_compatible(video_frame.format()))
        {

        }

        return false;
    }

    bool on_sink_frame_message(const i_message_frame& message_frame)
    {
        switch(message_frame.frame().media_type())
        {
            case media_type_t::video:
            {
                return on_sink_video_frame(message_frame.options()
                                           , static_cast<const i_video_frame&>(message_frame.frame()));
            }
            break;
            default:;
        }

        return false;
    }

    bool on_sink_message(const i_message& message)
    {
        switch(message.category())
        {
            case message_category_t::frame:
                return on_sink_frame_message(static_cast<const i_message_frame&>(message));
            break;
            default:;
        }

        return false;
    }

    // i_media_transcoder interface
public:

    const i_media_format &format() const override
    {
        return m_output_format;
    }

    // i_message_source interface
public:
    bool add_sink(i_message_sink *sink) override
    {
        return m_source_router.add_sink(sink);
    }

    bool remove_sink(i_message_sink *sink) override
    {
        return m_source_router.remove_sink(sink);
    }

    // i_message_sink interface
public:
    bool send_message(const i_message &message) override
    {
        return on_sink_message(message);
    }
};


video_transcoder_factory::video_transcoder_factory()
{

}

i_media_transcoder::u_ptr_t video_transcoder_factory::create_transcoder(const i_property &property)
{
    return video_transcoder::create(property);
}



}
