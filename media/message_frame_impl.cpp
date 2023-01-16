#include "message_frame_impl.h"

namespace mpl::media
{

message_frame_base_impl::message_frame_base_impl(const i_option& options)
{
    m_options.merge(options);
}

message_frame_base_impl::message_frame_base_impl(option_impl &&options)
    : m_options(std::move(options))
{

}

option_impl &message_frame_base_impl::get_options()
{
    return m_options;
}

const option_impl &message_frame_base_impl::get_options() const
{
    return m_options;
}

void message_frame_base_impl::set_options(const i_option& options)
{
    m_options.assign(options);
}

void message_frame_base_impl::set_options(option_impl &&options)
{
    m_options = std::move(options);
}

message_category_t message_frame_base_impl::category() const
{
    return message_category_t::frame;
}

const i_option &message_frame_base_impl::options() const
{
    return m_options;
}


message_frame_ptr_impl::u_ptr_t message_frame_ptr_impl::create(const i_media_frame::s_ptr_t &media_frame
                                                               , const i_option& options)
{
    return std::make_unique<message_frame_ptr_impl>(media_frame
                                                    , options);
}

message_frame_ptr_impl::u_ptr_t message_frame_ptr_impl::create(const i_media_frame::s_ptr_t &media_frame
                                                               , option_impl &&options)
{
    return std::make_unique<message_frame_ptr_impl>(media_frame
                                                    , std::move(options));
}

message_frame_ptr_impl::message_frame_ptr_impl(const i_media_frame::s_ptr_t &media_frame
                                               , const i_option& options)
    : message_frame_base_impl(options)
    , m_frame(media_frame)
{

}

message_frame_ptr_impl::message_frame_ptr_impl(const i_media_frame::s_ptr_t &media_frame
                                               , option_impl &&options)
    : message_frame_base_impl(std::move(options))
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
            return create(std::move(clone_frame)
                          , options());
        }
    }

    return nullptr;
}

const i_media_frame &message_frame_ptr_impl::frame() const
{
    return *m_frame;
}

message_frame_ref_impl::message_frame_ref_impl(const i_media_frame &media_frame
                                               , const i_option& options)
    : message_frame_base_impl(options)
    , m_frame(media_frame)
{

}

message_frame_ref_impl::message_frame_ref_impl(const i_media_frame &media_frame
                                               , option_impl &&options)
    : message_frame_base_impl(std::move(options))
    , m_frame(media_frame)
{

}

i_message::u_ptr_t message_frame_ref_impl::clone() const
{
    return message_frame_ptr_impl::create(m_frame.clone()
                                          , options());
}

const i_media_frame &message_frame_ref_impl::frame() const
{
    return m_frame;
}



}
