#include "message_frame_impl.h"

namespace mpl
{

message_frame_base_impl::message_frame_base_impl(const stream_info_t &stream_info)
    : m_stream_info(stream_info)
{

}

void message_frame_base_impl::set_stream_info(const stream_info_t &stream_info)
{
    m_stream_info = stream_info;
}

message_category_t message_frame_base_impl::category() const
{
    return message_category_t::frame;
}

const stream_info_t &message_frame_base_impl::stream_info() const
{
    return m_stream_info;
}

message_frame_ptr_impl::u_ptr_t message_frame_ptr_impl::create(const stream_info_t &stream_info
                                                               , const i_media_frame::s_ptr_t &media_frame)
{
    return std::make_unique<message_frame_ptr_impl>(stream_info
                                                    , media_frame);
}

message_frame_ptr_impl::message_frame_ptr_impl(const stream_info_t &stream_info
                                               , const i_media_frame::s_ptr_t &media_frame)
    : message_frame_base_impl(stream_info)
    , m_frame(media_frame)
{

}

void message_frame_ptr_impl::set_frame(const i_media_frame::s_ptr_t &media_frame)
{
    m_frame = media_frame;
}

i_message::u_ptr_t message_frame_ptr_impl::clone() const
{
    if (m_frame)
    {
        if (auto clone_frame = m_frame->clone())
        {
            return create(stream_info()
                          , std::move(clone_frame));
        }
    }

    return nullptr;
}

const i_media_frame &message_frame_ptr_impl::frame() const
{
    return *m_frame;
}

message_frame_ref_impl::message_frame_ref_impl(const stream_info_t &stream_info
                                               , const i_media_frame &media_frame)
    : message_frame_base_impl(stream_info)
    , m_frame(media_frame)
{

}

i_message::u_ptr_t message_frame_ref_impl::clone() const
{
    return message_frame_ptr_impl::create(stream_info()
                                          , m_frame.clone());
}

const i_media_frame &message_frame_ref_impl::frame() const
{
    return m_frame;
}



}
