#ifndef MPL_APP_COMMAND_MESSAGE_FACTORY_IMPL_H
#define MPL_APP_COMMAND_MESSAGE_FACTORY_IMPL_H

#include "core/i_command_factory.h"

namespace mpl::app
{

class command_message_factory_impl : public i_message_command_factory
{
public:
    command_message_factory_impl() = default;

    // i_command_message_factory interface
public:

    static command_message_factory_impl& get_instance();

    i_message_command::u_ptr_t create_massage(const command_t &command) override;
};

}

#endif // MPL_APP_COMMAND_MESSAGE_FACTORY_IMPL_H
