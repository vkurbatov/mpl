#ifndef MPL_NET_UDP_TRANSPORT_FACTORY_H
#define MPL_NET_UDP_TRANSPORT_FACTORY_H

#include "net/i_transport_factory.h"

namespace pt::io
{

class io_core;

}

namespace mpl::net
{

// class net_engine_impl;

class udp_transport_factory : public i_transport_factory
{
    pt::io::io_core&        m_io_core;

public:

    using u_ptr_t = std::unique_ptr<i_transport_factory>;

    static u_ptr_t create(pt::io::io_core& io_core);

    udp_transport_factory(pt::io::io_core& io_core);

    // i_transport_factory interface
public:
    i_transport_channel::u_ptr_t create_transport(const i_property &params) override;
};

}

#endif // MPL_NET_UDP_TRANSPORT_FACTORY_H
