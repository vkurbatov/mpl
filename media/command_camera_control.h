#ifndef MPL_COMMAND_CAMERA_CONTROL_H
#define MPL_COMMAND_CAMERA_CONTROL_H

#include "core/command.h"
#include "core/i_property.h"

namespace mpl
{

struct command_camera_control_t : public command_t
{
    enum class state_t
    {
        request,
        success,
        failed
    };

    constexpr static command_id_t       id = 0;
    constexpr static std::string_view   command_name = "device_control";

    i_property::s_ptr_t                 commands;
    std::uint32_t                       control_id;
    state_t                             state;

public:

    command_camera_control_t(const i_property::s_ptr_t& commands = nullptr
                            , std::uint32_t control_id = 0
                            , state_t state = state_t::request);

};

}

#endif // MPL_COMMAND_CAMERA_CONTROL_H
