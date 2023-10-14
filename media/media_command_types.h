#ifndef MPL_MEDIA_COMMAND_TYPES_H
#define MPL_MEDIA_COMMAND_TYPES_H

#include "core/command_types.h"

namespace mpl::media
{

constexpr command_id_t media_base_command_id = core_base_command_id + 1000;
constexpr command_id_t camera_control_command_id = media_base_command_id + 0;

}

#endif // MPL_MEDIA_COMMAND_TYPES_H
