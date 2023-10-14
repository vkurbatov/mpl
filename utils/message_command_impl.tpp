#include "message_command_impl.h"

namespace mpl
{

template<typename Command, message_subclass_t Subclass>
typename message_command_impl<Command, Subclass>::u_ptr_t message_command_impl<Command, Subclass>::create(const Command &command)
{
    return std::make_unique<message_command_impl>(command);
}

template<typename MediaCommand, message_subclass_t Subclass>
typename message_command_impl<MediaCommand, Subclass>::u_ptr_t message_command_impl<MediaCommand, Subclass>::create(MediaCommand &&command)
{
    return std::make_unique<message_command_impl>(std::move(command));
}

template<typename MediaCommand, message_subclass_t Subclass>
message_command_impl<MediaCommand, Subclass>::message_command_impl(const MediaCommand &command)
    : m_command(command)
{

}

template<typename MediaCommand, message_subclass_t Subclass>
message_command_impl<MediaCommand, Subclass>::message_command_impl(MediaCommand &&command)
    : m_command(std::move(command))
{

}

template<typename MediaCommand, message_subclass_t Subclass>
message_category_t message_command_impl<MediaCommand, Subclass>::category() const
{
    return message_category_t::command;
}

template<typename MediaCommand, message_subclass_t Subclass>
message_subclass_t message_command_impl<MediaCommand, Subclass>::subclass() const
{
    return Subclass;
}

template<typename MediaCommand, message_subclass_t Subclass>
i_message::u_ptr_t message_command_impl<MediaCommand, Subclass>::clone() const
{
    return create(m_command);
}

template<typename MediaCommand, message_subclass_t Subclass>
const command_t &message_command_impl<MediaCommand, Subclass>::command() const
{
    return m_command;
}

}
