#include "video_frame_impl.h"

namespace mpl
{

video_frame_base_impl::video_frame_base_impl(frame_id_t frame_id
                                             , timestamp_t timestamp
                                             , i_video_frame::frame_type_t frame_type)
    : m_frame_id(frame_id)
    , m_timestamp(timestamp)
    , m_frame_type(frame_type)
{

}

void video_frame_base_impl::set_frame_id(frame_id_t frame_id)
{
    m_frame_id = frame_id;
}

void video_frame_base_impl::set_timestamp(timestamp_t timestamp)
{
    m_timestamp = timestamp;
}

void video_frame_base_impl::set_buffers(smart_buffer_collection &&buffers)
{
    m_buffers = std::move(buffers);
}

void video_frame_base_impl::set_frame_type(frame_type_t frame_type)
{
    m_frame_type = frame_type;
}

smart_buffer_collection &video_frame_base_impl::buffers()
{
    return m_buffers;
}

const smart_buffer_collection &video_frame_base_impl::buffers() const
{
    return m_buffers;
}

media_type_t video_frame_base_impl::media_type() const
{
    return media_type_t::video;
}

const i_buffer *video_frame_base_impl::get_buffer(int32_t buffer_index) const
{
    return m_buffers.get_buffer(buffer_index);
}

std::size_t video_frame_base_impl::buffers_count() const
{
    return m_buffers.count();
}

frame_id_t video_frame_base_impl::frame_id() const
{
    return m_frame_id;
}

timestamp_t video_frame_base_impl::timestamp() const
{
    return m_timestamp;
}


i_video_frame::frame_type_t video_frame_base_impl::frame_type() const
{
    return m_frame_type;
}


video_frame_impl::u_ptr_t video_frame_impl::create(const video_format_impl &video_format
                                                   , frame_id_t frame_id
                                                   , timestamp_t timestamp
                                                   , i_video_frame::frame_type_t frame_type)
{
    return std::make_unique<video_frame_impl>(video_format
                                              , frame_id
                                              , timestamp
                                              , frame_type);
}

video_frame_impl::u_ptr_t video_frame_impl::create(video_format_impl &&video_format
                                                   , frame_id_t frame_id
                                                   , timestamp_t timestamp
                                                   , i_video_frame::frame_type_t frame_type)
{
    return std::make_unique<video_frame_impl>(std::move(video_format)
                                              , frame_id
                                              , timestamp
                                              , frame_type);
}

video_frame_impl::video_frame_impl(const video_format_impl &video_format
                                   , frame_id_t frame_id
                                   , timestamp_t timestamp
                                   , i_video_frame::frame_type_t frame_type)
    : video_frame_base_impl(frame_id
                            , timestamp
                            , frame_type)
    , m_video_format(video_format)
{

}

video_frame_impl::video_frame_impl(video_format_impl &&video_format
                                   , frame_id_t frame_id
                                   , timestamp_t timestamp
                                   , i_video_frame::frame_type_t frame_type)
    : video_frame_base_impl(frame_id
                            , timestamp
                            , frame_type)
    , m_video_format(std::move(video_format))
{

}

void video_frame_impl::set_format(const video_format_impl &video_format)
{
    m_video_format = video_format;
}

void video_frame_impl::set_format(video_format_impl &&video_format)
{
    m_video_format = std::move(video_format);
}

i_media_frame::u_ptr_t video_frame_impl::clone() const
{
    if (auto clone_frame = create(m_video_format
                                  , m_frame_id
                                  , m_timestamp
                                  , m_frame_type))
    {
        clone_frame->m_buffers = m_buffers.fork();
        return clone_frame;
    }

    return nullptr;
}

const i_video_format &video_frame_impl::format() const
{
    return m_video_format;
}

video_frame_ptr_impl::u_ptr_t video_frame_ptr_impl::create(const i_video_format::s_ptr_t &video_format
                                                           , frame_id_t frame_id
                                                           , timestamp_t timestamp
                                                           , frame_type_t frame_type)
{
    return std::make_unique<video_frame_ptr_impl>(video_format
                                                  , frame_id
                                                  , timestamp
                                                  , frame_type);
}

video_frame_ptr_impl::video_frame_ptr_impl(const i_video_format::s_ptr_t &video_format
                                           , frame_id_t frame_id
                                           , timestamp_t timestamp
                                           , frame_type_t frame_type)
    : video_frame_base_impl(frame_id
                            , timestamp
                            , frame_type)
    , m_video_format_ptr(video_format)
{

}

void video_frame_ptr_impl::set_format(const i_video_format::s_ptr_t &video_format)
{
    m_video_format_ptr = video_format;
}

i_media_frame::u_ptr_t video_frame_ptr_impl::clone() const
{
    if (m_video_format_ptr)
    {
        if (i_media_format::s_ptr_t clone_format = m_video_format_ptr->clone())
        {
            if (auto clone_frame = create(std::static_pointer_cast<i_video_format>(clone_format)
                                          , m_frame_id
                                          , m_timestamp
                                          , m_frame_type))
            {
                clone_frame->m_buffers = m_buffers.fork();
                return clone_frame;
            }
        }
    }

    return nullptr;
}

const i_video_format &video_frame_ptr_impl::format() const
{
    return *m_video_format_ptr;
}

video_frame_ref_impl::video_frame_ref_impl(const i_video_format &video_format
                                           , frame_id_t frame_id
                                           , timestamp_t timestamp
                                           , i_video_frame::frame_type_t frame_type)
    : video_frame_base_impl(frame_id
                            , timestamp
                            , frame_type)
    , m_video_format(video_format)
{

}

i_media_frame::u_ptr_t video_frame_ref_impl::clone() const
{
    if (i_media_format::s_ptr_t clone_format = format().clone())
    {
        if (auto clone_frame = video_frame_ptr_impl::create(std::static_pointer_cast<i_video_format>(clone_format)
                                                           , m_frame_id
                                                           , m_timestamp
                                                           , m_frame_type))
        {
            clone_frame->set_buffers(m_buffers.fork());
            return clone_frame;
        }
    }

    return nullptr;
}

const i_video_format &video_frame_ref_impl::format() const
{
    return m_video_format;
}


}
