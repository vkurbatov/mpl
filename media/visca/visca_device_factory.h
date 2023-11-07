#ifndef MPL_MEDIA_VISCA_DEVICE_FACTORY_H
#define MPL_MEDIA_VISCA_DEVICE_FACTORY_H

#include "media/i_device_factory.h"


namespace mpl::net
{

class i_transport_factory;

}

namespace mpl::media
{

class visca_device_factory : public i_device_factory
{
    net::i_transport_factory&   m_serial_factory;
public:
    using u_ptr_t = std::unique_ptr<visca_device_factory>;
    using s_ptr_t = std::shared_ptr<visca_device_factory>;

    static u_ptr_t create(net::i_transport_factory& transport_factory);

    visca_device_factory(net::i_transport_factory& transport_factory);

    // i_device_factory interface
public:
    i_device::u_ptr_t create_device(const i_property &device_params) override;
};
}

#endif // MPL_MEDIA_VISCA_DEVICE_FACTORY_H
