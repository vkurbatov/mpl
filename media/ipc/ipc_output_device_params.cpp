#include "ipc_output_device_params.h"

namespace mpl::media
{

ipc_output_device_params_t::ipc_output_device_params_t(const std::string_view &channel_name
                                                       , std::size_t buffer_size)
    : channel_name(channel_name)
    , buffer_size(buffer_size)
{

}

bool ipc_output_device_params_t::is_valid() const
{
    return !channel_name.empty()
                && buffer_size > 0;
}

}
