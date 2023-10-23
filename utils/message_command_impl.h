#ifndef MPL_UTILS_MESSAGE_COMMAND_IMPL_H
#define MPL_UTILS_MESSAGE_COMMAND_IMPL_H

#include "core/i_message_command.h"

namespace mpl
{

template<typename Command, message_subclass_t Subclass>
class message_command_impl : public i_message_command
{
    Command    m_command;
public:

    using u_ptr_t = std::unique_ptr<message_command_impl>;
    using s_ptr_t = std::shared_ptr<message_command_impl>;

    static u_ptr_t create(const Command& command);
    static u_ptr_t create(Command&& command);

    message_command_impl(const Command& command);
    message_command_impl(Command&& command = {});

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

#endif // MPL_UTILS_MESSAGE_COMMAND_IMPL_H
