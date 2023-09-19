#ifndef MPL_COMMAND_CAMERA_CONTROL_H
#define MPL_COMMAND_CAMERA_CONTROL_H

#include "core/command.h"
#include "core/i_property.h"

namespace mpl
{

struct command_camera_control_t : public command_t
{
    constexpr static command_id_t       id = 0;
    constexpr static std::string_view   command_name = "device_control";

    i_property::s_ptr_t                 commands;
    std::uint32_t                       control_id;

public:

    command_camera_control_t(const i_property::s_ptr_t& commands = nullptr
                            , std::uint32_t control_id = 0);

};

}

#endif // MPL_COMMAND_CAMERA_CONTROL_H
