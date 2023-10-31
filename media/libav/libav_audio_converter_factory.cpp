#include "libav_audio_converter_factory.h"

#include "utils/convert_utils.h"
#include "utils/property_reader.h"
#include "utils/option_helper.h"
#include "core/i_buffer_collection.h"

#include "media/audio_format_impl.h"
#include "media/audio_frame_impl.h"

#include "tools/ffmpeg/libav_resampler.h"

namespace mpl::media
{

namespace detail
{

bool audio_info_from_format(const i_audio_format& format
                            , pt::ffmpeg::audio_info_t& audio_info)
{
    pt::ffmpeg::stream_info_t stream_info;
    if (utils::convert(format
                       , stream_info))
    {
        audio_info = stream_info.media_info.audio_info;
        return true;
    }

    return false;
}

}

class libav_audio_converter : public i_media_converter
{
    pt::ffmpeg::libav_resampler     m_native_resampler;
    audio_format_impl           m_input_format;
    audio_format_impl           m_output_format;
    i_message_sink*             m_output_sink;

    pt::ffmpeg::audio_info_t        m_input_audio_info;
    pt::ffmpeg::audio_info_t        m_output_audio_info;

public:
    using u_ptr_t = std::unique_ptr<libav_audio_converter>;

    static u_ptr_t create(const i_property &params)
    {
        property_reader reader(params);
        audio_format_impl audio_format;
        if (reader.get("format"
                       , audio_format))
        {
            if (audio_format.media_type() == media_type_t::audio
                    && audio_format.is_valid()
                    && audio_format.is_convertable())
            {
                return std::make_unique<libav_audio_converter>(std::move(audio_format));

            }
        }

        return nullptr;
    }

    libav_audio_converter(audio_format_impl &&output_format)
        : m_output_format(std::move(output_format))
    {
        detail::audio_info_from_format(m_output_format
                                       , m_output_audio_info);

    }

    bool check_or_update_format(const i_audio_format& audio_format)
    {
        if (audio_format.is_valid()
                && audio_format.is_convertable())
        {
            if (!audio_format.is_compatible(m_input_format))
            {
                m_input_format.assign(audio_format);
                return detail::audio_info_from_format(m_input_format
                                                      , m_input_audio_info);
            }
            else
            {
                return true;
            }
        }

        return false;
    }

    bool on_audio_frame(const i_audio_frame& audio_frame)
    {
        if (m_output_sink != nullptr
                && check_or_update_format(audio_frame.format()))
        {
            if (auto buffer = audio_frame.data().get_buffer(media_buffer_index))
            {
                auto output_samples = m_native_resampler.resample(m_input_audio_info
                                                                  , buffer->data()
                                                                  , buffer->size()
                                                                  , m_output_audio_info);

                if (!output_samples.empty())
                {
                    audio_frame_impl converted_audio_frame(m_output_format
                                                           , audio_frame.frame_id()
                                                           , audio_frame.timestamp());

                    converted_audio_frame.set_ntp_timestamp(audio_frame.ntp_timestamp());
                    converted_audio_frame.set_options(audio_frame.options());

                    converted_audio_frame.smart_buffers().set_buffer(media_buffer_index
                                                                    , smart_buffer(std::move(output_samples)));

                    return m_output_sink->send_message(converted_audio_frame);
                }
            }
        }

        return false;
    }
    // i_message_sink interface
public:
    bool send_message(const i_message &message) override
    {
        if (message.category() == message_category_t::data
                && m_output_sink != nullptr)
        {
            const i_media_frame& media_frame = static_cast<const i_media_frame&>(message);
            if (media_frame.media_type() == media_type_t::audio)
            {
                const auto& audio_frame = static_cast<const i_audio_frame&>(media_frame);
                if (audio_frame.format().is_compatible(m_output_format))
                {
                    return m_output_sink->send_message(message);
                }
                return on_audio_frame(audio_frame);
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

libav_audio_converter_factory &libav_audio_converter_factory::get_instance()
{
    static libav_audio_converter_factory single_audio_converter_factory;
    return single_audio_converter_factory;
}

libav_audio_converter_factory::libav_audio_converter_factory()
{

}

i_media_converter::u_ptr_t libav_audio_converter_factory::create_converter(const i_property &params)
{
    return libav_audio_converter::create(params);
}


}
