#ifndef MPL_DEVICE_TYPES_H
#define MPL_DEVICE_TYPES_H

namespace mpl
{

enum class device_type_t
{
    undefined = -1,
    v4l2,
    file,
    http,
    rtsp,
    rtmp,
    vnc
};

}

#endif // MPL_DEVICE_TYPES_H
