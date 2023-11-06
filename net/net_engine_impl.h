#ifndef MPL_NET_ENGINE_IMPL_H
#define MPL_NET_ENGINE_IMPL_H

#include "i_net_engine.h"

namespace pt::io
{

class io_core;

}

namespace mpl
{

class i_task_manager;

namespace net
{

struct net_engine_config_t;

class net_engine_impl : public i_net_engine
{

private:

    struct pimpl_t;
    using pimpl_ptr_t = std::unique_ptr<pimpl_t>;

    pimpl_ptr_t         m_pimpl;

public:


    using u_ptr_t = std::unique_ptr<net_engine_impl>;
    using s_ptr_t = std::shared_ptr<net_engine_impl>;

    static u_ptr_t create(const net_engine_config_t& config
                          , i_task_manager& task_manager
                          , i_timer_manager& timer_manager);

    net_engine_impl(const net_engine_config_t& config
                    , i_task_manager& task_manager
                    , i_timer_manager& timer_manager);

    ~net_engine_impl();

    pt::io::io_core& io_core();

    // i_net_engine interface
public:

    bool start() override;
    bool stop() override;
    bool is_started() const;


    // i_net_engine interface
public:
    i_task_manager &task_manager() override;
    i_timer_manager &timer_manager() override;
    i_transport_factory *transport_factory(transport_id_t transport_id) override;
    i_net_packet_builder::u_ptr_t create_packet_builder(transport_id_t transport_id) override;
};

}

}

#endif // MPL_NET_ENGINE_IMPL_H
