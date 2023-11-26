#include "media_frame_builder_impl.h"
#include "audio_frame_impl.h"
#include "video_frame_impl.h"

namespace mpl::media
{

namespace detail
{

i_media_format::u_ptr_t get_media_format(const i_media_frame& media_frame)
{
    switch(media_frame.media_type())
    {
        case media_type_t::audio:
            return static_cast<const i_audio_frame&>(media_frame).format().clone();
        break;
        case media_type_t::video:
            return static_cast<const i_video_frame&>(media_frame).format().clone();
        break;
        default:;
    }

    return nullptr;
}

video_frame_type_t get_frame_type(const i_media_frame& media_frame)
{
    switch(media_frame.media_type())
    {
        case media_type_t::video:
            return static_cast<const i_video_frame&>(media_frame).frame_type();
        break;
        default:;
    }

    return video_frame_type_t::undefined;
}

}

media_frame_builder_impl::u_ptr_t media_frame_builder_impl::create()
{
    return std::make_unique<media_frame_builder_impl>();
}

media_frame_builder_impl::u_ptr_t media_frame_builder_impl::create(const i_media_frame &media_frame)
{
    return std::make_unique<media_frame_builder_impl>(media_frame);
}

media_frame_builder_impl::media_frame_builder_impl()
    : m_frame_type(video_frame_type_t::undefined)
{

}

media_frame_builder_impl::media_frame_builder_impl(const i_media_frame &media_frame)
    : media_frame_builder_impl()
{
    assign(media_frame);
}

void media_frame_builder_impl::assign(const i_media_frame &media_frame)
{
    m_frame_info.frame_id = media_frame.frame_id();
    m_frame_info.timestamp = media_frame.timestamp();
    m_frame_info.ntp_timestamp = media_frame.ntp_timestamp();
    m_frame_type = detail::get_frame_type(media_frame);
    m_media_format = detail::get_media_format(media_frame);
    m_options.assign(media_frame.options());
    m_buffers.assign(media_frame.data());
}

void media_frame_builder_impl::set_frame_id(frame_id_t frame_id)
{
    m_frame_info.frame_id = frame_id;
}

frame_id_t media_frame_builder_impl::frame_id() const
{
    return m_frame_info.frame_id;
}

void media_frame_builder_impl::set_timestamp(timestamp_t timestamp)
{
    m_frame_info.timestamp = timestamp;
}

timestamp_t media_frame_builder_impl::timestamp() const
{
    return m_frame_info.timestamp;
}

void media_frame_builder_impl::set_ntp_timestamp(timestamp_t ntp_timestamp)
{
    m_frame_info.ntp_timestamp = ntp_timestamp;
}

timestamp_t media_frame_builder_impl::ntp_timestamp() const
{
    return m_frame_info.ntp_timestamp;
}

void media_frame_builder_impl::set_frame_type(video_frame_type_t frame_type)
{
    m_frame_type = frame_type;
}

video_frame_type_t media_frame_builder_impl::frame_type() const
{
    return m_frame_type;
}

void media_frame_builder_impl::set_frame_buffer(std::int32_t buffer_index
                                                , const void *frame_data
                                                , std::size_t frame_size)
{
    if (auto buffer = m_buffers.get_smart_buffer(buffer_index))
    {
        buffer->assign(frame_data
                       , frame_size
                       , true);

    }
}

const i_buffer *media_frame_builder_impl::frame_buffer(std::int32_t buffer_index)
{
    return m_buffers.get_buffer(buffer_index);
}

void media_frame_builder_impl::set_frame_option(const i_option &frame_options)
{
    m_options.assign(frame_options);
}

i_option &media_frame_builder_impl::frame_options()
{
    return m_options;
}

void media_frame_builder_impl::set_format(const i_media_format &media_format)
{
    m_media_format = media_format.clone();
}

const i_media_format *media_frame_builder_impl::media_format() const
{
    return m_media_format.get();
}

i_media_frame::u_ptr_t media_frame_builder_impl::create_frame()
{
    if (m_media_format)
    {
        switch(m_media_format->media_type())
        {
            case media_type_t::audio:
            {
                if (auto audio_frame = audio_frame_impl::create(static_cast<const i_audio_format&>(*m_media_format)
                                                                , m_frame_info.frame_id
                                                                , m_frame_info.timestamp))
                {
                    audio_frame->set_ntp_timestamp(m_frame_info.ntp_timestamp);
                    audio_frame->smart_buffers().assign(m_buffers);

                    return audio_frame;
                }
            }
            break;
            case media_type_t::video:
            {
                if (auto video_frame = video_frame_impl::create(static_cast<const i_video_format&>(*m_media_format)
                                                                , m_frame_info.frame_id
                                                                , m_frame_info.timestamp
                                                                , m_frame_type))
                {
                    video_frame->set_ntp_timestamp(m_frame_info.ntp_timestamp);
                    video_frame->smart_buffers().assign(m_buffers);

                    return video_frame;
                }
            }
            break;
            default:;
        }
    }
    return nullptr;
}


}
