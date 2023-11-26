#ifndef MPL_I_APP_MODULE_H
#define MPL_I_APP_MODULE_H

#include "core/i_module.h"

namespace mpl
{

class i_message_event_factory;
class i_message_command_factory;

namespace app
{

class i_app_module : public i_module
{
public:
    using u_ptr_t = std::unique_ptr<i_app_module>;
    using s_ptr_t = std::shared_ptr<i_app_module>;

    virtual i_message_event_factory& events() = 0;
    virtual i_message_command_factory& commands() = 0;
};

}

}

#endif // MPL_I_APP_MODULE_H
