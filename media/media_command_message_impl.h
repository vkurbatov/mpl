#ifndef MPL_MEDIA_COMMAND_MESSAGE_IMPL_H
#define MPL_MEDIA_COMMAND_MESSAGE_IMPL_H

#include "core/i_message_command.h"

namespace mpl::media
{

template<typename MediaCommand>
class media_command_message_impl : public i_message_command
{
    MediaCommand    m_command;
public:

    using u_ptr_t = std::unique_ptr<media_command_message_impl>;
    using s_ptr_t = std::shared_ptr<media_command_message_impl>;

    static u_ptr_t create(const MediaCommand& command);
    static u_ptr_t create(MediaCommand&& command);

    media_command_message_impl(const MediaCommand& command);
    media_command_message_impl(MediaCommand&& command);

    // i_message interface
public:
    message_category_t category() const override;
    message_subclass_t subclass() const override;
    i_message::u_ptr_t clone() const override;

    // i_message_command interface
public:
    const command_t &command() const override;
};

}

#endif // MPL_MEDIA_COMMAND_MESSAGE_IMPL_H
