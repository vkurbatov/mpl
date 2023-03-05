#include "libav_video_converter_factory.h"

#include "core/property_reader.h"
#include "core/convert_utils.h"
#include "core/option_helper.h"
#include "core/i_buffer_collection.h"

#include "video_format_impl.h"
#include "video_frame_impl.h"

#include "message_frame_impl.h"

#include "tools/ffmpeg/libav_converter.h"

namespace mpl::media
{


namespace detail
{
    bool fragment_info_from_format(const i_video_format& format
                                   , ffmpeg::fragment_info_t& fragment_info)
    {
        ffmpeg::stream_info_t stream_info;
        if (core::utils::convert(format
                                 , stream_info))
        {
            fragment_info.pixel_format = stream_info.media_info.video_info.pixel_format;
            fragment_info.frame_size = stream_info.media_info.video_info.size;
            fragment_info.frame_rect.size = fragment_info.frame_size;

            return true;
        }

        return false;
    }

}

class libav_video_converter : public i_media_converter
{
    ffmpeg::libav_converter     m_native_converter;
    video_format_impl           m_input_format;
    video_format_impl           m_output_format;
    i_message_sink*             m_output_sink;

    ffmpeg::fragment_info_t     m_input_fragment_info;
    ffmpeg::fragment_info_t     m_output_fragment_info;
    raw_array_t                 m_output_buffer;

public:
    using u_ptr_t = std::unique_ptr<libav_video_converter>;

    static u_ptr_t create(const i_property& params)
    {
        property_reader reader(params);
        video_format_impl video_format;
        if (reader.get("format", video_format))
        {
            if (video_format.is_valid()
                    && video_format.is_convertable())
            {
                return std::make_unique<libav_video_converter>(std::move(video_format));
            }
        }

        return nullptr;
    }

    libav_video_converter(video_format_impl &&video_format)
        : m_output_format(video_format)
    {
        detail::fragment_info_from_format(m_output_format
                                          , m_output_fragment_info);

        m_output_buffer.resize(m_output_fragment_info.get_frame_size());

    }

    bool check_or_update_format(const i_video_format& video_format)
    {
        if (video_format.is_valid()
                && video_format.is_convertable())
        {
            if (!video_format.is_compatible(m_input_format))
            {
                m_input_format.assign(video_format);
                if (detail::fragment_info_from_format(m_input_format
                                                      , m_input_fragment_info))
                {
                    return true;
                }
            }
            else
            {
                return true;
            }

        }

        return false;
    }

    bool on_video_frame(const i_video_frame& video_frame)
    {
        if (m_output_sink != nullptr
                && check_or_update_format(video_frame.format()))
        {
            if (auto buffer = video_frame.buffers().get_buffer(main_media_buffer_index))
            {
                auto input_fragment_info = m_input_fragment_info;

                ffmpeg::frame_rect_t::aspect_ratio(m_output_fragment_info.frame_rect
                                                   , input_fragment_info.frame_rect);

                if (m_native_converter.convert_frames(input_fragment_info
                                                      , buffer->data()
                                                      , m_output_fragment_info
                                                      , m_output_buffer.data()))
                {

                    video_frame_impl converted_video_frame(m_output_format
                                                           , video_frame.frame_id()
                                                           , video_frame.timestamp()
                                                           , video_frame.frame_type());

                    converted_video_frame.smart_buffers().set_buffer(main_media_buffer_index
                                                                     , smart_buffer(m_output_buffer.data()
                                                                     , m_output_buffer.size()));

                    return m_output_sink->send_message(message_frame_ref_impl(converted_video_frame));
                }
            }
        }

        return false;
    }
    // i_message_sink interface
public:
    bool send_message(const i_message &message) override
    {
        if (message.category() == message_category_t::frame)
        {
            const i_message_frame& message_frame = static_cast<const i_message_frame&>(message);
            if (message_frame.frame().media_type() == media_type_t::video)
            {
                return on_video_frame(static_cast<const i_video_frame&>(message_frame.frame()));
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

libav_video_converter_factory::libav_video_converter_factory()
{

}

i_media_converter::u_ptr_t libav_video_converter_factory::create_converter(const i_property& params)
{
    return libav_video_converter::create(params);
}



}
