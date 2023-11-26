#include "command_message_factory_impl.h"
#include "utils/message_command_impl.h"
#include "media/command_camera_control.h"
#include "media/media_module_types.h"
#include "net/ice/ice_gathering_command.h"
#include "net/net_module_types.h"

namespace mpl::app
{

namespace detail
{

template<module_id_t ModuleId, typename Command>
i_message_command::u_ptr_t create_command_message(const Command& command)
{
    return message_command_impl<Command, ModuleId>::create(command);
}

}

command_message_factory_impl &command_message_factory_impl::get_instance()
{
    static command_message_factory_impl single_factory;
    return single_factory;
}

i_message_command::u_ptr_t command_message_factory_impl::create_massage(const command_t &command)
{
    switch(command.command_id)
    {
        case media::command_camera_control_t::id:
            return detail::create_command_message<media::media_module_id>(static_cast<const media::command_camera_control_t&>(command));
        break;
        case net::ice_gathering_command_t::id:
            return detail::create_command_message<net::net_module_id>(static_cast<const net::ice_gathering_command_t&>(command));
        break;
        default:;
    }

    return nullptr;
}



}
