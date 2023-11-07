#ifndef MPL_I_COMMAND_FACTORY_H
#define MPL_I_COMMAND_FACTORY_H

#include "i_message_command.h"

namespace mpl
{

class i_command_factory
{
public:
    using u_ptr_t = std::unique_ptr<i_command_factory>;
    using s_ptr_t = std::shared_ptr<i_command_factory>;

    virtual ~i_command_factory() = default;

    virtual i_message_command::u_ptr_t create_massage(const command_t& command) = 0;
};

}

#endif // MPL_I_COMMAND_FACTORY_H
