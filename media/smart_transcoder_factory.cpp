#include "smart_transcoder_factory.h"

#include "message_frame_impl.h"

#include "audio_frame_impl.h"
#include "video_frame_impl.h"

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

template<typename Format>
bool is_compatible_format(const Format& input_format
                          , const Format& output_format)
{
    return input_format.is_compatible(output_format);
}

class converter_manager
{
    i_media_converter_factory&  m_media_decoders;
    i_media_converter_factory&  m_media_encoders;
    i_media_converter_factory&  m_media_converters;

public:
    converter_manager(i_media_converter_factory& media_decoders
                      , i_media_converter_factory& media_encoders
                      , i_media_converter_factory& media_converters)
        : m_media_decoders(media_decoders)
        , m_media_encoders(media_encoders)
        , m_media_converters(media_converters)
    {

    }

    i_media_converter::u_ptr_t create_decoder(const i_media_format& encode_format
                                              , i_message_sink* sink)
    {
        if (auto decoder = m_media_decoders.create_converter(encode_format))
        {
            decoder->set_sink(sink);
            return decoder;
        }

        return nullptr;
    }
    i_media_converter::u_ptr_t create_encoder(const i_media_format& decode_format
                                              , i_message_sink* sink)
    {
        if (auto encoder = m_media_encoders.create_converter(decode_format))
        {
            encoder->set_sink(sink);
            return encoder;
        }

        return nullptr;

    }
    i_media_converter::u_ptr_t create_converter(const i_media_format& convert_format
                                                , i_message_sink* sink)
    {
        if (auto converter = m_media_converters.create_converter(convert_format))
        {
            converter->set_sink(sink);
            return converter;
        }

        return nullptr;

    }
};

template<media_type_t MediaType>
class frame_sink : public i_message_sink
{
    using i_frame_t = typename format_types_t<MediaType>::i_frame_t;
    using frame_handler_t = std::function<bool(const i_frame_t& frame, const i_option& options)>;

    frame_handler_t m_frame_handler;

public:
    frame_sink(frame_handler_t&& frame_handler)
        : m_frame_handler(frame_handler)
    {

    }

    // i_message_sink interface
public:
    bool send_message(const i_message &message) override
    {
        if (message.category() == message_category_t::frame)
        {
            const auto& message_frame = static_cast<const i_message_frame&>(message);
            if (message_frame.frame().media_type() == MediaType)
            {

                return m_frame_handler(static_cast<const i_frame_t&>(message_frame.frame())
                                       , message_frame.options());
            }
        }

        return false;
    }
};

}

template<media_type_t MediaType>
class smart_transcoder : public i_media_converter
{
    using i_format_t = typename detail::format_types_t<MediaType>::i_format_t;
    using i_frame_t = typename detail::format_types_t<MediaType>::i_frame_t;
    using format_impl_t = typename detail::format_types_t<MediaType>::format_impl_t;
    using frame_impl_t = typename detail::format_types_t<MediaType>::frame_impl_t;

    detail::converter_manager       m_converter_manager;

    format_impl_t                   m_input_format;
    format_impl_t                   m_output_format;

    i_media_converter::u_ptr_t      m_decoder;
    i_media_converter::u_ptr_t      m_encoder;
    i_media_converter::u_ptr_t      m_converter;

    detail::frame_sink<MediaType>   m_decoder_sink;
    detail::frame_sink<MediaType>   m_encoder_sink;
    detail::frame_sink<MediaType>   m_converter_sink;

    i_message_sink*                 m_output_sink;

    bool                            m_is_init;

public:
    using u_ptr_t = std::unique_ptr<smart_transcoder>;

    static u_ptr_t create(const i_format_t &output_format
                          , i_media_converter_factory &media_decoders
                          , i_media_converter_factory &media_encoders
                          , i_media_converter_factory &media_converters)
    {
        return std::make_unique<smart_transcoder>(output_format
                                                 , media_decoders
                                                 , media_encoders
                                                 , media_converters);
    }


    smart_transcoder(const i_format_t &output_format
                     , i_media_converter_factory &media_decoders
                     , i_media_converter_factory &media_encoders
                     , i_media_converter_factory &media_converters)
        : m_converter_manager(media_decoders
                              , media_encoders
                              , media_converters)
        , m_output_format(output_format)
        , m_decoder_sink([&](const auto& frame, const auto& options)
                        { return on_decoder_frame(frame, options); } )
        , m_encoder_sink([&](const auto& frame, const auto& options)
                        { return on_encoder_frame(frame, options); } )
        , m_converter_sink([&](const auto& frame, const auto& options)
                        { return on_converter_frame(frame, options); } )
        , m_output_sink(nullptr)
        , m_is_init(false)
    {

    }

    bool check_and_init_converters(const i_format_t& input_format
                                   , i_format_t& output_format)
    {
        if (!detail::is_compatible_format(input_format
                                          , output_format))
        {
            i_format_t input_converted_format(input_format);
            i_format_t output_convert_format(output_format);

            if (output_format.is_encoded())
            {
                if (m_encoder == nullptr
                        || !m_encoder->output_format().is_compatible(output_format))
                {
                    m_encoder.reset();
                    m_encoder = m_converter_manager.create_encoder(output_format);

                    if (!m_encoder)
                    {
                        return false;
                    }

                    output_convert_format.assign(static_cast<const i_format_t&>(m_encoder->input_format()));
                }
            }
            else
            {
                m_encoder.reset();
            }
        }
        else
        {

        }

        return true;
    }

    bool on_message_frame(const i_frame_t& media_frame)
    {
        if (!m_input_format.is_compatible(media_frame.format()))
        {

        }

        return false;
    }


    bool on_encoder_frame(const i_frame_t& frame
                          , const i_option& options)
    {
        return false;
    }

    bool on_decoder_frame(const i_frame_t& frame
                          , const i_option& options)
    {
        return false;
    }

    bool on_converter_frame(const i_frame_t& frame
                           , const i_option& options)
    {
        return false;
    }

    void reset()
    {
        m_decoder.reset();
        m_encoder.reset();
        m_converter.reset();
        m_is_init = false;
    }

    // i_message_sink interface
public:
    bool send_message(const i_message &message) override
    {
        if (message.category() == message_category_t::frame)
        {
            const auto& frame = static_cast<const i_message_frame&>(message).frame();
            if (frame.media_type() == MediaType)
            {
                return on_message_frame(static_cast<const i_frame_t&>(frame));
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

smart_transcoder_factory::smart_transcoder_factory(i_media_converter_factory &media_decoders
                                                   , i_media_converter_factory &media_encoders
                                                   , i_media_converter_factory &media_converters)
    : m_media_decoders(media_decoders)
    , m_media_encoders(media_encoders)
    , m_media_converters(media_converters)
{

}

i_media_converter::u_ptr_t smart_transcoder_factory::create_converter(const i_media_format &convert_format)
{
    switch(convert_format.media_type())
    {
        case media_type_t::audio:
            return smart_transcoder<media_type_t::audio>::create(static_cast<const i_audio_format&>(convert_format)
                                                                , m_media_decoders
                                                                , m_media_encoders
                                                                , m_media_converters);
        break;
        case media_type_t::video:
            return smart_transcoder<media_type_t::video>::create(static_cast<const i_video_format&>(convert_format)
                                                                , m_media_decoders
                                                                , m_media_encoders
                                                                , m_media_converters);
        break;
        default:;
    }

    return nullptr;
}



}
