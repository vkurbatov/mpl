#include "net_core.h"
#include "utils/task_manager_impl.h"
#include "utils/timer_manager_impl.h"

#include "net_core_config.h"
#include "net/net_engine_impl.h"
#include "socket/udp_transport_factory.h"
#include "tls/tls_transport_factory.h"
#include "ice/ice_transport_factory.h"

namespace mpl::net
{

namespace detail
{

inline i_task_manager* select_task_manager(i_task_manager* task_manager)
{
    return task_manager == nullptr
                   ? &task_manager_factory::single_manager()
                   : task_manager;
}

inline i_timer_manager* select_timer_manager(i_timer_manager* timer_manager)
{
    return timer_manager == nullptr
                   ? &timer_manager_factory::single_manager()
                   : timer_manager;
}

}

struct net_core_impl
{
    using u_ptr_t = std::unique_ptr<net_core_impl>;

    net_core_config_t           m_config;
    i_task_manager*             m_task_manager;
    i_timer_manager*            m_timer_manager;
    net_engine_impl             m_net_engine;

    static u_ptr_t create(const net_core_config_t &config
                          , i_task_manager *task_manger
                          , i_timer_manager *timer_manager)
    {
        return std::make_unique<net_core_impl>(config
                                               , task_manger
                                               , timer_manager);
    }


    net_core_impl(const net_core_config_t &config
                  , i_task_manager *task_manger
                  , i_timer_manager *timer_manager)
        : m_task_manager(detail::select_task_manager(task_manger))
        , m_timer_manager(detail::select_timer_manager(timer_manager))
        , m_net_engine({config.net_workers}
                       , *m_task_manager)
    {

        start();
    }

    ~net_core_impl()
    {
        stop();
    }

    inline void start()
    {
        m_net_engine.start();
    }

    inline void stop()
    {
        m_net_engine.stop();
    }
};


net_core &net_core::get_instance()
{
    static net_core single_net_core;
    return single_net_core;
}

bool net_core::init(const net_core_config_t &config
                    , i_task_manager *task_manger
                    , i_timer_manager *timer_manager)
{
    return false;
}

bool net_core::cleanup()
{
    return false;
}

bool net_core::is_init() const
{
    return false;
}

i_timer_manager &net_core::timer_manager()
{

}

i_task_manager &net_core::task_manager()
{

}

i_transport_factory::u_ptr_t net_core::create_udp_factory()
{

}

i_transport_factory::u_ptr_t net_core::create_ice_factory(const ice_config_t &ice_config
                                                          , i_transport_factory *socket_factory
                                                          , i_timer_manager *timer_manager)
{

}

i_transport_factory::u_ptr_t net_core::create_tls_factory(const tls_config_t &tls_config)
{

}

i_net_packet::u_ptr_t net_core::create_packet(transport_id_t transport_id
                                              , const void *data
                                              , std::size_t size
                                              , bool copy
                                              , const socket_address_t &address)
{

}

i_net_packet::u_ptr_t net_core::create_packet(transport_id_t transport_id
                                              , i_data_object::s_ptr_t shared_data
                                              , const socket_address_t &address)
{

}



}
