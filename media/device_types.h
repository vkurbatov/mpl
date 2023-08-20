#ifndef MPL_MEDIA_DEVICE_TYPES_H
#define MPL_MEDIA_DEVICE_TYPES_H

namespace mpl::media
{

enum class device_type_t
{
    undefined = -1,
    v4l2_in,
    v4l2_out,
    libav_in,
    libav_out,
    vnc,
    ipc_in,
    ipc_out,
    apm,
    custom
};

}

#endif // MPL_MEDIA_DEVICE_TYPES_H
