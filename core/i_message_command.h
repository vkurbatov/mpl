#ifndef MPL_I_MESSAGE_COMMAND_H
#define MPL_I_MESSAGE_COMMAND_H

#include "i_message.h"

namespace mpl
{

struct command_t;

class i_message_command : public i_message
{
public:
    using u_ptr_t = std::unique_ptr<i_message_command>;
    using s_ptr_t = std::shared_ptr<i_message_command>;

    virtual const command_t& command() const = 0;
};

}


#endif // MPL_I_MESSAGE_COMMAND_H
