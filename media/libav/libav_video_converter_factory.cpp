#include "libav_video_converter_factory.h"

#include "utils/property_reader.h"
#include "utils/convert_utils.h"
#include "utils/option_helper.h"
#include "core/i_buffer_collection.h"

#include "media/video_format_impl.h"
#include "media/video_frame_impl.h"

#include "tools/ffmpeg/libav_converter.h"

#include "log/log_tools.h"

namespace mpl::media
{


namespace detail
{
    bool fragment_info_from_format(const i_video_format& format
                                   , pt::ffmpeg::fragment_info_t& fragment_info)
    {
        pt::ffmpeg::stream_info_t stream_info;
        if (utils::convert(format
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
    pt::ffmpeg::libav_converter     m_native_converter;
    video_format_impl               m_input_format;
    video_format_impl               m_output_format;
    i_message_sink*                 m_output_sink;

    pt::ffmpeg::fragment_info_t     m_input_fragment_info;
    pt::ffmpeg::fragment_info_t     m_output_fragment_info;
    raw_array_t                     m_output_buffer;

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
        : m_output_format(std::move(video_format))
    {
        mpl_log_info("libav video converter #", this, ": init {", m_output_format.info().to_string(), "}");

        detail::fragment_info_from_format(m_output_format
                                          , m_output_fragment_info);

        m_output_buffer.resize(m_output_fragment_info.get_frame_size());

    }

    ~libav_video_converter()
    {
        mpl_log_info("libav video converter #", this, ": destruction");
    }

    bool check_or_update_format(const i_video_format& video_format)
    {
        if (video_format.is_valid()
                && video_format.is_convertable())
        {
            if (!video_format.is_compatible(m_input_format))
            {
                m_input_format.assign(video_format);

                mpl_log_info("libav video converter #", this, ": update input format: ", m_input_format.info().to_string());

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
        if (check_or_update_format(video_frame.format()))
        {
            if (auto buffer = video_frame.data().get_buffer(media_buffer_index))
            {
                auto input_fragment_info = m_input_fragment_info;

                pt::ffmpeg::frame_rect_t::aspect_ratio(m_output_fragment_info.frame_rect
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

                    converted_video_frame.set_ntp_timestamp(video_frame.ntp_timestamp());
                    converted_video_frame.set_options(video_frame.options());

                    converted_video_frame.smart_buffers().set_buffer(media_buffer_index
                                                                     , smart_buffer(m_output_buffer.data()
                                                                     , m_output_buffer.size()));

                    mpl_log_debug("libav video converter #", this, ": send conversion frame #", converted_video_frame.frame_id());

                    return m_output_sink->send_message(converted_video_frame);
                }
            }
            else
            {
                mpl_log_warning("libav video converter #", this, ": nothing frame data buffer");
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
            if (media_frame.media_type() == media_type_t::video)
            {
                const auto& video_frame = static_cast<const i_video_frame&>(media_frame);
                if (video_frame.format().is_compatible(m_output_format))
                {
                    mpl_log_debug("libav video converter #", this, ": transit frame #", video_frame.frame_id());
                    return m_output_sink->send_message(message);
                }
                return on_video_frame(video_frame);
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

libav_video_converter_factory &libav_video_converter_factory::get_instance()
{
    static libav_video_converter_factory single_video_converter_factory;
    return single_video_converter_factory;
}

libav_video_converter_factory::libav_video_converter_factory()
{

}

i_media_converter::u_ptr_t libav_video_converter_factory::create_converter(const i_property& params)
{
    return libav_video_converter::create(params);
}



}
