#include "utils/message_command_impl.tpp"
#include "command_camera_control.h"

#include "media_types.h"

namespace mpl
{

template class message_command_impl<media::command_camera_control_t, media::message_media_class>;


}
