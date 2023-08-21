#ifndef MPL_APM_DEVICE_FACTORY_H
#define MPL_APM_DEVICE_FACTORY_H

#include "i_device_factory.h"

namespace mpl::media
{

class apm_device_factory : public i_device_factory
{
public:
    using u_ptr_t = std::unique_ptr<apm_device_factory>;
    using s_ptr_t = std::shared_ptr<apm_device_factory>;

    static u_ptr_t create();

    apm_device_factory();


    // i_device_factory interface
public:
    i_device::u_ptr_t create_device(const i_property &device_params) override;
};

}

#endif // MPL_APM_DEVICE_FACTORY_H
