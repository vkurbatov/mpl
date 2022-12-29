#include "audio_frame_impl.h"

namespace mpl
{

audio_frame_base_impl::audio_frame_base_impl(frame_id_t frame_id
                                           , timestamp_t timestamp)
    : m_frame_id(frame_id)
    , m_timestamp(timestamp)
{

}

void audio_frame_base_impl::set_frame_id(frame_id_t frame_id)
{
    m_frame_id = frame_id;
}

void audio_frame_base_impl::set_timestamp(timestamp_t timestamp)
{
    m_timestamp = timestamp;
}

void audio_frame_base_impl::set_buffers(smart_buffer_collection &&buffers)
{
    m_buffers = std::move(buffers);
}

smart_buffer_collection &audio_frame_base_impl::buffers()
{
    return m_buffers;
}

const smart_buffer_collection &audio_frame_base_impl::buffers() const
{
    return m_buffers;
}

media_type_t audio_frame_base_impl::media_type() const
{
    return media_type_t::audio;
}

const i_buffer *audio_frame_base_impl::get_buffer(int32_t buffer_index) const
{
    return m_buffers.get_buffer(buffer_index);
}

std::size_t audio_frame_base_impl::buffers_count() const
{
    return m_buffers.count();
}

frame_id_t audio_frame_base_impl::frame_id() const
{
    return m_frame_id;
}

timestamp_t audio_frame_base_impl::timestamp() const
{
    return m_timestamp;
}

audio_frame_impl::u_ptr_t audio_frame_impl::create(const audio_format_impl &audio_format
                                                   , frame_id_t frame_id
                                                   , timestamp_t timestamp)
{
    return std::make_unique<audio_frame_impl>(audio_format
                                              , frame_id
                                              , timestamp);
}

audio_frame_impl::u_ptr_t audio_frame_impl::create(audio_format_impl &&audio_format
                                                   , frame_id_t frame_id
                                                   , timestamp_t timestamp)
{
    return std::make_unique<audio_frame_impl>(std::move(audio_format)
                                              , frame_id
                                              , timestamp);
}

audio_frame_impl::audio_frame_impl(const audio_format_impl &audio_format
                                   , frame_id_t frame_id
                                   , timestamp_t timestamp)
    : audio_frame_base_impl(frame_id
                           , timestamp)
    , m_audio_format(audio_format)
{

}

audio_frame_impl::audio_frame_impl(audio_format_impl &&audio_format
                                   , frame_id_t frame_id
                                   , timestamp_t timestamp)
    : audio_frame_base_impl(frame_id
                           , timestamp)
    , m_audio_format(std::move(audio_format))
{

}

void audio_frame_impl::set_format(const audio_format_impl &audio_format)
{
    m_audio_format = audio_format;
}

void audio_frame_impl::set_format(audio_format_impl &&audio_format)
{
    m_audio_format = std::move(audio_format);
}

i_media_frame::u_ptr_t audio_frame_impl::clone() const
{
    if (auto clone_frame = create(m_audio_format
                                  , m_frame_id
                                  , m_timestamp))
    {
        clone_frame->m_buffers = m_buffers.fork();
        return clone_frame;
    }

    return nullptr;
}

const i_audio_format &audio_frame_impl::format() const
{
    return m_audio_format;
}

audio_frame_ptr_impl::u_ptr_t audio_frame_ptr_impl::create(const i_audio_format::s_ptr_t &audio_format
                                                           , frame_id_t frame_id
                                                           , timestamp_t timestamp)
{
    return std::make_unique<audio_frame_ptr_impl>(audio_format
                                                  , frame_id
                                                  , timestamp);
}

audio_frame_ptr_impl::audio_frame_ptr_impl(const i_audio_format::s_ptr_t &audio_format
                                           , frame_id_t frame_id
                                           , timestamp_t timestamp)
    : audio_frame_base_impl(frame_id
                            , timestamp)
    , m_audio_format_ptr(audio_format)
{

}

void audio_frame_ptr_impl::set_format(const i_audio_format::s_ptr_t &audio_format)
{
    m_audio_format_ptr = audio_format;
}

i_media_frame::u_ptr_t audio_frame_ptr_impl::clone() const
{
    if (m_audio_format_ptr)
    {
        if (i_media_format::s_ptr_t clone_format = m_audio_format_ptr->clone())
        {
            if (auto clone_frame = create(std::static_pointer_cast<i_audio_format>(clone_format)
                                          , m_frame_id
                                          , m_timestamp))
            {
                clone_frame->m_buffers = m_buffers.fork();
                return clone_frame;
            }
        }
    }

    return nullptr;
}

const i_audio_format &audio_frame_ptr_impl::format() const
{
    return *m_audio_format_ptr;
}

audio_frame_ref_impl::audio_frame_ref_impl(const i_audio_format &audio_format
                                           , frame_id_t frame_id
                                           , timestamp_t timestamp)
    : audio_frame_base_impl(frame_id
                            , timestamp)
    , m_audio_format(audio_format)
{

}

i_media_frame::u_ptr_t audio_frame_ref_impl::clone() const
{
    if (i_media_format::s_ptr_t clone_format = format().clone())
    {
        if (auto clone_frame = audio_frame_ptr_impl::create(std::static_pointer_cast<i_audio_format>(clone_format)
                                                           , m_frame_id
                                                           , m_timestamp))
        {
            clone_frame->set_buffers(m_buffers.fork());
            return clone_frame;
        }
    }

    return nullptr;
}

const i_audio_format &audio_frame_ref_impl::format() const
{
    return m_audio_format;
}

}
