#ifndef MPL_MEDIA_IPC_OUTPUT_DEVICE_PARAMS_H
#define MPL_MEDIA_IPC_OUTPUT_DEVICE_PARAMS_H

#include <string>

namespace mpl::media
{

struct ipc_output_device_params_t
{
    std::string         channel_name;
    std::size_t         buffer_size;

    ipc_output_device_params_t(const std::string_view& channel_name = {}
                               , std::size_t buffer_size = 0);

    bool is_valid() const;
};

}

#endif // MPL_MEDIA_IPC_OUTPUT_DEVICE_PARAMS_H
