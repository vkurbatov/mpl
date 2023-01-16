#ifndef MPL_I_DEVICE_FACTORY_H
#define MPL_I_DEVICE_FACTORY_H

#include "i_device.h"

namespace mpl
{

class i_property;

namespace media
{

class i_device_factory
{
public:
    using u_ptr_t = std::unique_ptr<i_device_factory>;
    using s_ptr_t = std::shared_ptr<i_device_factory>;

    virtual ~i_device_factory() = default;
    virtual i_device::u_ptr_t create_device(const i_property& device_params) = 0;
};

}

}

#endif // MPL_I_DEVICE_FACTORY_H
