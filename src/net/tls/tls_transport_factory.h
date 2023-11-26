#ifndef MPL_NET_TLS_TRANSPORT_FACTORY_H
#define MPL_NET_TLS_TRANSPORT_FACTORY_H

#include "core/i_timer_manager.h"
#include "net/i_transport_factory.h"
#include "tls_config.h"

namespace mpl::net
{

class tls_transport_factory : public i_transport_factory
{
    struct pimpl_t;
    using pimpl_ptr_t = std::unique_ptr<pimpl_t>;

    pimpl_ptr_t         m_pimpl;

public:

    using u_ptr_t = std::unique_ptr<tls_transport_factory>;
    using s_ptr_t = std::shared_ptr<tls_transport_factory>;

    static u_ptr_t create(const tls_config_t& tls_config
                          , i_timer_manager& timer_manager);

    tls_transport_factory(const tls_config_t& tls_config
                          , i_timer_manager& timer_manager);

    ~tls_transport_factory();

    // i_transport_factory interface
public:
    i_transport_channel::u_ptr_t create_transport(const i_property &params) override;
};

}

#endif // MPL_NET_TLS_TRANSPORT_FACTORY_H
