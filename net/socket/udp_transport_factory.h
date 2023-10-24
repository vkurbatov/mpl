#ifndef MPL_NET_UDP_TRANSPORT_FACTORY_H
#define MPL_NET_UDP_TRANSPORT_FACTORY_H

#include "net/i_transport_factory.h"


namespace mpl::net
{

class net_engine_impl;

class udp_transport_factory : public i_transport_factory
{
    net_engine_impl&            m_engine;

public:

    using u_ptr_t = std::unique_ptr<i_transport_factory>;

    static u_ptr_t create(net_engine_impl& engine);

    udp_transport_factory(net_engine_impl& engine);

    // i_transport_factory interface
public:
    i_transport_channel::u_ptr_t create_transport(const i_property &params) override;
};

}

#endif // MPL_NET_UDP_TRANSPORT_FACTORY_H
