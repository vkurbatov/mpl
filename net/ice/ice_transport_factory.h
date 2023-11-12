#ifndef MPL_NET_ICE_TRANSPORT_FACTORY_H
#define MPL_NET_ICE_TRANSPORT_FACTORY_H

#include "core/i_timer_manager.h"
#include "net/i_transport_factory.h"
#include "ice_config.h"

namespace mpl::net
{

class ice_transport_factory : public i_transport_factory
{
    ice_config_t                m_ice_config;
    i_transport_factory&        m_socket_factory;
    i_timer_manager&            m_timer_manager;
public:

    using u_ptr_t = std::unique_ptr<ice_transport_factory>;
    using s_ptr_t = std::shared_ptr<ice_transport_factory>;

    static u_ptr_t create(const ice_config_t& ice_config
                          , i_transport_factory& socket_factory
                          , i_timer_manager& timer_manager);

    ice_transport_factory(const ice_config_t& ice_config
                          , i_transport_factory& socket_factory
                          , i_timer_manager& timer_manager);

    // i_transport_factory interface
public:
    i_transport_channel::u_ptr_t create_transport(const i_property &params) override;
};

}

#endif // MPL_NET_ICE_TRANSPORT_FACTORY_H
