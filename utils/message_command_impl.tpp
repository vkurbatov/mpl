#include "message_command_impl.h"

namespace mpl
{

template<typename Command, module_id_t ModuleId>
typename message_command_impl<Command, ModuleId>::u_ptr_t message_command_impl<Command, ModuleId>::create(const Command &command)
{
    return std::make_unique<message_command_impl>(command);
}

template<typename MediaCommand, module_id_t ModuleId>
typename message_command_impl<MediaCommand, ModuleId>::u_ptr_t message_command_impl<MediaCommand, ModuleId>::create(MediaCommand &&command)
{
    return std::make_unique<message_command_impl>(std::move(command));
}

template<typename MediaCommand, module_id_t ModuleId>
message_command_impl<MediaCommand, ModuleId>::message_command_impl(const MediaCommand &command)
    : m_command(command)
{

}

template<typename MediaCommand, module_id_t ModuleId>
message_command_impl<MediaCommand, ModuleId>::message_command_impl(MediaCommand &&command)
    : m_command(std::move(command))
{

}

template<typename MediaCommand, module_id_t ModuleId>
message_category_t message_command_impl<MediaCommand, ModuleId>::category() const
{
    return message_category_t::command;
}

template<typename MediaCommand, module_id_t ModuleId>
module_id_t message_command_impl<MediaCommand, ModuleId>::module_id() const
{
    return ModuleId;
}

template<typename MediaCommand, module_id_t ModuleId>
i_message::u_ptr_t message_command_impl<MediaCommand, ModuleId>::clone() const
{
    return create(m_command);
}

template<typename MediaCommand, module_id_t ModuleId>
const command_t &message_command_impl<MediaCommand, ModuleId>::command() const
{
    return m_command;
}

}
