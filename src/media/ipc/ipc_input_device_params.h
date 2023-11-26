#ifndef MPL_MEDIA_IPC_INPUT_DEVICE_PARAMS_H
#define MPL_MEDIA_IPC_INPUT_DEVICE_PARAMS_H

#include <string>

namespace mpl::media
{

struct ipc_input_device_params_t
{
    std::string         channel_name;
    ipc_input_device_params_t(const std::string_view& channel_name = {});

    bool is_valid() const;
};

}

#endif // MPL_MEDIA_IPC_INPUT_DEVICE_PARAMS_H
