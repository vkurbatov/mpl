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

#include "media_option_types.h"


#include "tools/ffmpeg/libav_converter.h"
#include "tools/ffmpeg/libav_transcoder.h"

namespace mpl::media
{


struct complex_transcoder
{    

    ffmpeg::libav_transcoder::u_ptr_t   m_native_decoder;
    ffmpeg::libav_converter::u_ptr_t    m_native_converter;
    ffmpeg::libav_transcoder::u_ptr_t   m_native_encoder;

    video_format_impl                   m_input_format;
    video_format_impl                   m_output_format;

    bool                                m_is_init;

    static ffmpeg::libav_converter::u_ptr_t create_converter()
    {
        return ffmpeg::libav_converter::create();
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

    complex_transcoder(const i_video_format& output_format)
        : m_output_format(output_format)
        , m_is_init(false)
    {

    }

    void reset()
    {
        m_is_init = false;
        m_native_decoder.reset();
        m_native_converter.reset();
        m_native_encoder.reset();
    }

    bool reinitialize()
    {
        if (m_input_format.is_compatible(m_output_format))
        {
            reset();
            return true;
        }

        ffmpeg::libav_transcoder::u_ptr_t   native_decoder = nullptr;
        ffmpeg::libav_converter::u_ptr_t    native_converter = nullptr;
        ffmpeg::libav_transcoder::u_ptr_t   native_encoder = nullptr;


        bool need_decoder = m_input_format.is_encoded();
        bool need_encoder = m_output_format.is_encoded();
        bool need_converter = m_output_format.is_convertable();

        if (need_encoder)
        {
            native_encoder = create_encoder(m_output_format);
            if (native_encoder == nullptr)
            {
                return false;
            }
        }

        if (need_decoder)
        {
            native_decoder = create_decoder(m_input_format);
            if (native_decoder == nullptr)
            {
                return false;
            }
        }

        if (need_converter == false)
        {
            //
        }

        return true;
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

        }

        return false;
    }

    bool push_libav_frame(const ffmpeg::stream_info_t& stream_info
                          , const void* data
                          , std::size_t size)
    {
        return false;
    }

    bool on_decoder_frame(const ffmpeg::stream_info_t& stream_info
                          , const void* data
                          , std::size_t)
    {
        return false;
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
