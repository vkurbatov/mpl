#ifndef MPL_MESSAGE_FRAME_IMPL_H
#define MPL_MESSAGE_FRAME_IMPL_H

#include "i_message_frame.h"
#include "i_media_frame.h"
#include "stream_info.h"
#include "core/option_impl.h"

namespace mpl::media
{

class message_frame_base_impl : public i_message_frame
{
    option_impl         m_options;
public:

    using u_ptr_t = std::unique_ptr<message_frame_base_impl>;
    using s_ptr_t = std::shared_ptr<message_frame_base_impl>;

    message_frame_base_impl(const i_option& options);
    message_frame_base_impl(option_impl&& options);

    option_impl& get_options();
    const option_impl& get_options() const;

    void set_options(const i_option& options);
    void set_options(option_impl&& options);

    // i_message interface
public:
    message_category_t category() const override;


    // i_message_frame interface
public:
    const i_option& options() const override;
};

class message_frame_ptr_impl : public message_frame_base_impl
{
    i_media_frame::s_ptr_t      m_frame;
public:

    using u_ptr_t = std::unique_ptr<message_frame_ptr_impl>;
    using s_ptr_t = std::shared_ptr<message_frame_ptr_impl>;

    static u_ptr_t create(const i_media_frame::s_ptr_t& media_frame
                          , const i_option& options);
    static u_ptr_t create(const i_media_frame::s_ptr_t& media_frame
                          , option_impl&& options = {});

    message_frame_ptr_impl(const i_media_frame::s_ptr_t& media_frame
                           , const i_option& options);

    message_frame_ptr_impl(const i_media_frame::s_ptr_t& media_frame
                           , option_impl&& options = {});

    void set_frame(const i_media_frame::s_ptr_t& media_frame);


    // i_message interface
public:
    i_message::u_ptr_t clone() const override;

    // i_message_frame interface
public:
    const i_media_frame &frame() const override;
};

class message_frame_ref_impl : public message_frame_base_impl
{
    const i_media_frame&        m_frame;
public:

    using u_ptr_t = std::unique_ptr<message_frame_ref_impl>;
    using s_ptr_t = std::shared_ptr<message_frame_ref_impl>;

    message_frame_ref_impl(const i_media_frame& media_frame
                           , const i_option& options);

    message_frame_ref_impl(const i_media_frame& media_frame
                           , option_impl&& options = {});


    // i_message interface
public:
    i_message::u_ptr_t clone() const override;

    // i_message_frame interface
public:
    const i_media_frame &frame() const override;
};



}

#endif // MPL_MESSAGE_FRAME_IMPL_H
