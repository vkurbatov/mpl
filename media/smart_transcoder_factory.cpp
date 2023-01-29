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

}

template<media_type_t MediaType>
class smart_transcoder : public i_media_converter
{
    using i_format_t = typename detail::format_types_t<MediaType>::i_format_t;
    using i_frame_t = typename detail::format_types_t<MediaType>::i_frame_t;
    using format_impl_t = typename detail::format_types_t<MediaType>::format_impl_t;
    using frame_impl_t = typename detail::format_types_t<MediaType>::frame_impl_t;

    format_impl_t               m_input_format;
    format_impl_t               m_output_format;

    i_message_sink*             m_output_sink;

    bool                        m_is_init;

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
    {

    }

    bool on_message_frame(const i_message_frame& frame_message)
    {
        return false;
    }

    // i_message_sink interface
public:
    bool send_message(const i_message &message) override
    {
        if (message.category() == message_category_t::frame)
        {
            return on_message_frame(static_cast<const i_message_frame&>(message));
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
