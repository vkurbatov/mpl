#ifndef MPL_MEDIA_VNC_DEVICE_FACTORY_H
#define MPL_MEDIA_VNC_DEVICE_FACTORY_H

#include "i_device_factory.h"

namespace mpl::media
{

class vnc_device_factory : public i_device_factory
{
public:
    using u_ptr_t = std::unique_ptr<vnc_device_factory>;
    using s_ptr_t = std::shared_ptr<vnc_device_factory>;

    static u_ptr_t create();

    vnc_device_factory();

    // i_device_factory interface
public:
    i_device::u_ptr_t create_device(const i_property &device_params) override;
};


}

#endif // MPL_MEDIA_VNC_DEVICE_FACTORY_H
