#include "audio_frame_impl.h"

namespace mpl
{

audio_frame_base_impl::audio_frame_base_impl(const i_audio_format *format_ref
                                           , frame_id_t frame_id)
    : m_audio_format_ref(format_ref)
    , m_frame_id(frame_id)
{

}

void audio_frame_base_impl::set_frame_id(frame_id_t frame_id)
{
    m_frame_id = frame_id;
}

void audio_frame_base_impl::set_buffer(int32_t buffer_index
                                                       , smart_buffer &&buffer)
{
    m_buffers[buffer_index] = std::move(buffer);
}

bool audio_frame_base_impl::remove_buffer(int32_t buffer_index)
{
    return m_buffers.erase(buffer_index) > 0;
}

void audio_frame_base_impl::make_shared_buffers()
{
    for (auto& b : m_buffers)
    {
        b.second.make_shared();
    }
}

void audio_frame_base_impl::set_format(const i_audio_format *format_ref)
{
    m_audio_format_ref = format_ref;
}

media_type_t audio_frame_base_impl::media_type() const
{
    return media_type_t::audio;
}

const i_buffer *audio_frame_base_impl::get_buffer(int32_t buffer_index) const
{
    auto it = m_buffers.find(buffer_index);
    return it != m_buffers.end()
            ? &it->second
            : nullptr;
}

std::size_t audio_frame_base_impl::buffers() const
{
    return m_buffers.size();
}

frame_id_t audio_frame_base_impl::frame_id() const
{
    return m_frame_id;
}

const i_audio_format &audio_frame_base_impl::format() const
{
    return *m_audio_format_ref;
}

audio_frame_impl::u_ptr_t audio_frame_impl::create(const audio_format_impl &audio_format
                                                   , frame_id_t frame_id)
{
    return std::make_unique<audio_frame_impl>(audio_format
                                              , frame_id);
}

audio_frame_impl::u_ptr_t audio_frame_impl::create(audio_format_impl &&audio_format
                                                   , frame_id_t frame_id)
{
    return std::make_unique<audio_frame_impl>(std::move(audio_format)
                                              , frame_id);
}

audio_frame_impl::audio_frame_impl(const audio_format_impl &audio_format
                                   , frame_id_t frame_id)
    : audio_frame_base_impl(&m_audio_format
                           , frame_id)
    , m_audio_format(audio_format)
{

}

audio_frame_impl::audio_frame_impl(audio_format_impl &&audio_format
                                   , frame_id_t frame_id)
    : audio_frame_base_impl(&m_audio_format
                           , frame_id)
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
                                  , m_frame_id))
    {
        clone_frame->m_buffers = m_buffers;
        clone_frame->make_shared_buffers();

        return clone_frame;
    }

    return nullptr;
}

audio_frame_ptr_impl::u_ptr_t audio_frame_ptr_impl::create(const i_audio_format::s_ptr_t &audio_format
                                                           , frame_id_t frame_id)
{
    return std::make_unique<audio_frame_ptr_impl>(audio_format
                                                  , frame_id);
}

audio_frame_ptr_impl::audio_frame_ptr_impl(const i_audio_format::s_ptr_t &audio_format
                                           , frame_id_t frame_id)
    : audio_frame_base_impl(audio_format.get()
                            , frame_id)
    , m_audio_format_ptr(audio_format)
{

}

void audio_frame_ptr_impl::set_format(const i_audio_format::s_ptr_t &audio_format)
{
    m_audio_format_ptr = std::move(audio_format);
    audio_frame_base_impl::set_format(m_audio_format_ptr.get());
}

i_media_frame::u_ptr_t audio_frame_ptr_impl::clone() const
{
    if (m_audio_format_ptr)
    {
        if (i_media_format::s_ptr_t clone_format = m_audio_format_ptr->clone())
        {
            if (auto clone_frame = create(std::static_pointer_cast<i_audio_format>(clone_format)
                                          , m_frame_id))
            {
                clone_frame->make_shared_buffers();
                return clone_frame;
            }
        }
    }

    return nullptr;
}

audio_frame_ref_impl::audio_frame_ref_impl(const i_audio_format &audio_format
                                           , frame_id_t frame_id)
    : audio_frame_base_impl(&audio_format
                            , frame_id)
{

}

i_media_frame::u_ptr_t audio_frame_ref_impl::clone() const
{
    if (i_media_format::s_ptr_t clone_format = format().clone())
    {
        if (auto clone_frame = audio_frame_ptr_impl::create(std::static_pointer_cast<i_audio_format>(clone_format)
                                                           , m_frame_id))
        {
            clone_frame->make_shared_buffers();
            return clone_frame;
        }
    }

    return nullptr;
}

}
