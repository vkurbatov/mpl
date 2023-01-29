#ifndef MPL_DEVICE_TYPES_H
#define MPL_DEVICE_TYPES_H

namespace mpl::media
{

enum class device_type_t
{
    undefined = -1,
    v4l2,
    libav,
    vnc,
    custom
};

}

#endif // MPL_DEVICE_TYPES_H
