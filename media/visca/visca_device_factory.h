#ifndef MPL_MEDIA_VISCA_DEVICE_FACTORY_H
#define MPL_MEDIA_VISCA_DEVICE_FACTORY_H

#include "media/i_device_factory.h"

namespace pt::io
{

class io_core;

}

namespace mpl::media
{

class visca_device_factory : public i_device_factory
{
    pt::io::io_core&        m_io_core;
public:
    using u_ptr_t = std::unique_ptr<visca_device_factory>;
    using s_ptr_t = std::shared_ptr<visca_device_factory>;

    static u_ptr_t create(pt::io::io_core& io_core);

    visca_device_factory(pt::io::io_core& io_core);

    // i_device_factory interface
public:
    i_device::u_ptr_t create_device(const i_property &device_params) override;
};
}

#endif // MPL_MEDIA_VISCA_DEVICE_FACTORY_H
