#ifndef MPL_NET_SERIAL_TRANSPORT_FACTORY_H
#define MPL_NET_SERIAL_TRANSPORT_FACTORY_H

#include "net/i_transport_factory.h"

namespace pt::io
{

class io_core;

}

namespace mpl::net
{

class serial_transport_factory : public i_transport_factory
{
    pt::io::io_core&        m_io_core;

public:

    using u_ptr_t = std::unique_ptr<serial_transport_factory>;

    static u_ptr_t create(pt::io::io_core& io_core);

    serial_transport_factory(pt::io::io_core& io_core);

    // i_transport_factory interface
public:
    i_transport_channel::u_ptr_t create_transport(const i_property &params) override;
};

}

#endif // MPL_NET_SERIAL_TRANSPORT_FACTORY_H
