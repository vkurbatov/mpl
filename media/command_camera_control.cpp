#include "command_camera_control.h"

namespace mpl
{

// command_t::command_id_t command_camera_control_t::id = command_t::register_command(command_camera_control_t::command_name);

command_camera_control_t::command_camera_control_t(const i_property::s_ptr_t& commands
                                               , std::uint32_t control_id)
    : command_t(id
                , command_name)
    , commands(commands)
    , control_id(control_id)
{

}


}
