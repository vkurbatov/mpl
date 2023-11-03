#include "ipc_input_device_params.h"

namespace mpl::media
{

ipc_input_device_params_t::ipc_input_device_params_t(const std::string_view &channel_name)
    : channel_name(channel_name)
{

}

bool ipc_input_device_params_t::is_valid() const
{
    return !channel_name.empty();
}



}
