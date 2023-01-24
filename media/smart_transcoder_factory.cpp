#include "smart_transcoder_factory.h"

#include "i_audio_frame.h"
#include "i_video_frame.h"
#include "message_frame_impl.h"

namespace mpl::media
{


class smart_transcoder : public i_media_converter
{
    i_media_format::u_ptr_t     m_input_format;
    i_media_format::u_ptr_t     m_output_format;

    bool                        m_is_init;

public:
    using u_ptr_t = std::unique_ptr<smart_transcoder>;

    static u_ptr_t create(const i_media_format &output_format
                          , i_media_converter_factory &media_decoders
                          , i_media_converter_factory &media_encoders
                          , i_media_converter_factory &media_converters)
    {
        return std::make_unique<smart_transcoder>(output_format
                                                 , media_decoders
                                                 , media_encoders
                                                 , media_converters);
    }


    smart_transcoder(const i_media_format &output_format
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
    const i_media_format &input_format() const override;
    const i_media_format &output_format() const override;
    void set_sink(i_message_sink *output_sink) override;
};

smart_transcoder_factory::smart_transcoder_factory(i_media_converter_factory &media_decoders
                                                   , i_media_converter_factory &media_encoders
                                                   , i_media_converter_factory &media_converters)
    : m_media_decoders(media_decoders)
    , m_media_encoders(media_encoders)
    , m_media_converters(media_converters)
{

}

i_media_converter::u_ptr_t smart_transcoder_factory::create_converter(const i_media_format &output_format)
{
    return smart_transcoder::create(output_format
                                    , m_media_decoders
                                    , m_media_encoders
                                    , m_media_converters);
}



}
