#ifndef MPL_MEDIA_LIBAV_OUTPUT_DEVICE_FACTORY_H
#define MPL_MEDIA_LIBAV_OUTPUT_DEVICE_FACTORY_H

#include "media/i_device_factory.h"

namespace mpl::media
{

class libav_output_device_factory : public i_device_factory
{
public:
    libav_output_device_factory();

    // i_device_factory interface
public:
    i_device::u_ptr_t create_device(const i_property &device_params) override;
};

}

#endif // MPL_MEDIA_LIBAV_OUTPUT_DEVICE_FACTORY_H
