#include "command_camera_control.h"

namespace mpl::media
{

command_camera_control_t::command_camera_control_t(const i_property::s_ptr_t& commands
                                                   , std::uint32_t control_id
                                                   , state_t state)
    : command_t(id
                , command_name)
    , commands(commands)
    , control_id(control_id)
    , state(state)
{

}


}
