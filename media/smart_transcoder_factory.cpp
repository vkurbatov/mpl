#include "smart_transcoder_factory.h"

#include "core/property_writer.h"
#include "core/option_helper.h"
#include "message_frame_impl.h"

#include "audio_frame_impl.h"
#include "video_frame_impl.h"

#include "media_option_types.h"

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

void merge_format(const i_audio_format& input_format
                  , audio_format_impl& output_format)
{
    if (!output_format.is_valid())
    {
        output_format.options().merge(input_format.options());
        if (output_format.format_id() == audio_format_id_t::undefined)
        {
            output_format.set_format_id(input_format.format_id());
        }

        if (output_format.sample_rate() == 0)
        {
            output_format.set_sample_rate(input_format.sample_rate());
        }

        if (output_format.channels() == 0)
        {
            output_format.set_channels(input_format.channels());
        }

    }
}

void merge_format(const i_video_format& input_format
                  , video_format_impl& output_format)
{
    if (!output_format.is_valid())
    {

        output_format.options().merge(input_format.options());
        if (output_format.format_id() == video_format_id_t::undefined)
        {
            output_format.set_format_id(input_format.format_id());
        }

        if (output_format.width() == 0)
        {
            output_format.set_width(input_format.width());
        }

        if (output_format.height() == 0)
        {
            output_format.set_height(input_format.height());
        }

        if (output_format.frame_rate() == 0.0f)
        {
            output_format.set_frame_rate(input_format.frame_rate());
        }

    }
}


template<media_type_t MediaType>
class converter_manager
{
    using format_impl_t = typename detail::format_types_t<MediaType>::format_impl_t;

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

    i_media_converter::u_ptr_t create_decoder(const format_impl_t& encode_format
                                              , i_message_sink* sink)
    {
        if (auto decoder_params = encode_format.get_params("format"))
        {
            if (auto decoder = m_media_decoders.create_converter(*decoder_params))
            {
                decoder->set_sink(sink);
                return decoder;
            }
        }

        return nullptr;
    }
    i_media_converter::u_ptr_t create_encoder(const format_impl_t& decode_format
                                              , i_message_sink* sink)
    {
        if (auto encoder_params = decode_format.get_params("format"))
        {
            if (auto encoder = m_media_encoders.create_converter(*encoder_params))
            {
                encoder->set_sink(sink);
                return encoder;
            }
        }

        return nullptr;

    }
    i_media_converter::u_ptr_t create_converter(const format_impl_t& convert_format
                                                , i_message_sink* sink)
    {
        if (auto converter_params = convert_format.get_params("format"))
        {
            if (auto converter = m_media_converters.create_converter(*converter_params))
            {
                converter->set_sink(sink);
                return converter;
            }
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

    enum class transcoder_state_t
    {
        input,
        decode,
        convert,
        encode,
        output
    };

    struct params_t
    {
        bool    transcode_always;
        params_t(bool transcode_always = false)
            : transcode_always(transcode_always)
        {

        }

        params_t(const i_property& params)
            : params_t()
        {
            load(params);
        }

        bool load(const i_property& params)
        {
            property_reader reader(params);
            return reader.get("transcode_always", transcode_always);
        }

        bool store(i_property& params) const
        {
            property_writer writer(params);
            return writer.set("transcode_always", transcode_always, false);
        }
    };

    detail::converter_manager<MediaType>    m_converter_manager;

    params_t                                m_params;

    format_impl_t                           m_input_format;
    format_impl_t                           m_output_format;
    format_impl_t                           m_real_output_format;

    i_media_converter::u_ptr_t              m_decoder;
    i_media_converter::u_ptr_t              m_encoder;
    i_media_converter::u_ptr_t              m_converter;

    detail::frame_sink<MediaType>           m_decoder_sink;
    detail::frame_sink<MediaType>           m_encoder_sink;
    detail::frame_sink<MediaType>           m_converter_sink;

    i_message_sink*                         m_output_sink;

    bool                                    m_is_init;
    bool                                    m_is_transit;

public:
    using u_ptr_t = std::unique_ptr<smart_transcoder>;

    static u_ptr_t create(const i_property &params
                          , i_media_converter_factory &media_decoders
                          , i_media_converter_factory &media_encoders
                          , i_media_converter_factory &media_converters)
    {
        property_reader reader(params);
        format_impl_t media_format;
        if (reader.get("format", media_format))
        {
            params_t internal_params(params);

            return std::make_unique<smart_transcoder>(std::move(media_format)
                                                      , std::move(internal_params)
                                                      , media_decoders
                                                      , media_encoders
                                                      , media_converters);
        }

        return nullptr;
    }


    smart_transcoder(format_impl_t&& output_format
                     , params_t&& params
                     , i_media_converter_factory &media_decoders
                     , i_media_converter_factory &media_encoders
                     , i_media_converter_factory &media_converters)
        : m_converter_manager(media_decoders
                              , media_encoders
                              , media_converters)
        , m_params(std::move(params))
        , m_output_format(std::move(output_format))
        , m_real_output_format(m_output_format)
        , m_decoder_sink([&](const auto& frame, const auto& options)
                        { return on_decoder_frame(frame, options); } )
        , m_encoder_sink([&](const auto& frame, const auto& options)
                        { return on_encoder_frame(frame, options); } )
        , m_converter_sink([&](const auto& frame, const auto& options)
                        { return on_converter_frame(frame, options); } )
        , m_output_sink(nullptr)
        , m_is_init(false)
        , m_is_transit(true)
    {

    }

    bool check_and_init_converters(const i_format_t& input_format
                                   , const i_format_t& output_format)
    {
        if (!detail::is_compatible_format(input_format
                                          , output_format)
                || m_params.transcode_always)
        {
            format_impl_t input_convert_format(input_format);
            format_impl_t output_convert_format(output_format);

            // encoder
            if (output_format.is_encoded())
            {
                if (m_encoder == nullptr
                        || !m_encoder->output_format().is_compatible(output_format))
                {
                    m_encoder.reset();
                    m_encoder = m_converter_manager.create_encoder(output_format
                                                                   , &m_encoder_sink);

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

            // decoder
            if (input_format.is_encoded())
            {
                if (m_decoder == nullptr
                        || !m_decoder->input_format().is_compatible(input_format))
                {
                    m_decoder.reset();
                    m_decoder = m_converter_manager.create_decoder(input_format
                                                                   , &m_decoder_sink);

                    if (!m_decoder)
                    {
                        return false;
                    }

                    input_convert_format.assign(static_cast<const i_format_t&>(m_decoder->output_format()));
                }
            }
            else
            {
                m_decoder.reset();
            }

            // converter
            if (input_convert_format.is_compatible(output_convert_format))
            {
                m_converter.reset();
            }
            else
            {
                m_converter.reset();
                m_converter = m_converter_manager.create_converter(output_convert_format
                                                                   , &m_converter_sink);

                if (!m_converter)
                {
                    return false;
                }
            }
        }
        else
        {
            reset_converters();
        }

        return true;
    }

    template<transcoder_state_t State = transcoder_state_t::input>
    bool convert_and_write_frame(const i_frame_t& frame)
    {
        switch(State)
        {
            case transcoder_state_t::input:
                return m_is_transit
                        ? convert_and_write_frame<transcoder_state_t::output>(frame)
                        : convert_and_write_frame<transcoder_state_t::decode>(frame);
            break;
            case transcoder_state_t::decode:
                return m_decoder != nullptr
                        ? m_decoder->send_message(message_frame_ref_impl(frame))
                        : convert_and_write_frame<transcoder_state_t::convert>(frame);
            break;
            case transcoder_state_t::convert:
                return m_converter != nullptr
                        ? m_converter->send_message(message_frame_ref_impl(frame))
                        : convert_and_write_frame<transcoder_state_t::encode>(frame);
            break;
            case transcoder_state_t::encode:
                return m_encoder != nullptr
                        ? m_encoder->send_message(message_frame_ref_impl(frame))
                        : convert_and_write_frame<transcoder_state_t::output>(frame);
            break;
            case transcoder_state_t::output:
                if (m_output_sink)
                {
                    return m_output_sink->send_message(message_frame_ref_impl(frame));
                }
            break;
            default:;
        }

        return false;
    }

    bool on_message_frame(const i_frame_t& media_frame)
    {
        if (!m_input_format.is_compatible(media_frame.format())
                || !m_is_init)
        {
            m_input_format.assign(media_frame.format());

            m_real_output_format = m_output_format;
            detail::merge_format(m_input_format
                                 , m_real_output_format);

            m_is_init = check_and_init_converters(m_input_format
                                                  , m_real_output_format);

            if (!m_is_init)
            {
                reset_converters();
            }

            m_is_transit = is_transit();
        }

        if (m_is_init)
        {
            convert_and_write_frame(media_frame);
        }

        return false;
    }

    bool on_decoder_frame(const i_frame_t& frame
                          , const i_option& options)
    {
        return convert_and_write_frame<transcoder_state_t::convert>(frame);
    }

    bool on_converter_frame(const i_frame_t& frame
                           , const i_option& options)
    {
        return convert_and_write_frame<transcoder_state_t::encode>(frame);
    }

    bool on_encoder_frame(const i_frame_t& frame
                          , const i_option& options)
    {
        return convert_and_write_frame<transcoder_state_t::output>(frame);
    }


    void reset_converters()
    {
        m_decoder.reset();
        m_encoder.reset();
        m_converter.reset();
    }

    void reset()
    {
        reset_converters();
        m_is_init = false;
        m_is_transit = true;
    }

    bool is_transit() const
    {
        return m_decoder == nullptr
                && m_converter == nullptr
                && m_encoder == nullptr;
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
        return m_real_output_format;
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

i_media_converter::u_ptr_t smart_transcoder_factory::create_converter(const i_property &params)
{
    property_reader reader(params);
    switch(reader.get<media_type_t>("format.media_type"
                                    , media_type_t::undefined))
    {
        case media_type_t::audio:
            return smart_transcoder<media_type_t::audio>::create(params
                                                                , m_media_decoders
                                                                , m_media_encoders
                                                                , m_media_converters);
        break;
        case media_type_t::video:
            return smart_transcoder<media_type_t::video>::create(params
                                                                , m_media_decoders
                                                                , m_media_encoders
                                                                , m_media_converters);
        break;
        default:;
    }

    return nullptr;
}



}
