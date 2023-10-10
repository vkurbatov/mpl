#include "media_command_message_impl.h"
#include "command_camera_control.h"

namespace mpl::media
{

template class media_command_message_impl<command_camera_control_t>;

template<typename MediaCommand>
typename media_command_message_impl<MediaCommand>::u_ptr_t media_command_message_impl<MediaCommand>::create(const MediaCommand &command)
{
    return std::make_unique<media_command_message_impl>(command);
}

template<typename MediaCommand>
typename media_command_message_impl<MediaCommand>::u_ptr_t media_command_message_impl<MediaCommand>::create(MediaCommand &&command)
{
    return std::make_unique<media_command_message_impl>(std::move(command));
}

template<typename MediaCommand>
media_command_message_impl<MediaCommand>::media_command_message_impl(const MediaCommand &command)
    : m_command(command)
{

}

template<typename MediaCommand>
media_command_message_impl<MediaCommand>::media_command_message_impl(MediaCommand &&command)
    : m_command(std::move(command))
{

}

template<typename MediaCommand>
message_category_t media_command_message_impl<MediaCommand>::category() const
{
    return message_category_t::command;
}

template<typename MediaCommand>
message_subtype_t media_command_message_impl<MediaCommand>::subtype() const
{
    return static_cast<message_subtype_t>(m_command.command_id);
}

template<typename MediaCommand>
i_message::u_ptr_t media_command_message_impl<MediaCommand>::clone() const
{
    return create(m_command);
}

template<typename MediaCommand>
const command_t &media_command_message_impl<MediaCommand>::command() const
{
    return m_command;
}



}
