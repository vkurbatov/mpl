#include "net_core.h"
#include "utils/task_manager_impl.h"
#include "utils/timer_manager_impl.h"
#include "utils/smart_buffer.h"

#include "net_core_config.h"
#include "net/net_engine_impl.h"
#include "socket/udp_transport_factory.h"
#include "socket/socket_packet_impl.h"
#include "tls/tls_transport_factory.h"
#include "tls/tls_packet_impl.h"
#include "ice/ice_transport_factory.h"
#include "stun/stun_packet_impl.h"

#include <atomic>

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

i_net_packet::u_ptr_t create_packet(transport_id_t transport_id
                                    , smart_buffer&& packet_data
                                    , const socket_address_t& address)
{
    switch(transport_id)
    {
        case transport_id_t::udp:
            return udp_packet_impl::create(std::move(packet_data)
                                           , address);
        break;
        case transport_id_t::tcp:
            return tcp_packet_impl::create(std::move(packet_data)
                                           , address);
        break;
        case transport_id_t::tls:
            return tls_packet_impl::create(std::move(packet_data)
                                           , address);
        break;
        case transport_id_t::ice:
            return stun_packet_impl::create(std::move(packet_data)
                                           , address);
        break;
        default:;
    }

    return nullptr;
}

}

struct net_core_impl
{
    using u_ptr_t = std::unique_ptr<net_core_impl>;

    static u_ptr_t              m_single_core;
    static std::atomic_bool     m_init;

    net_core_config_t           m_config;
    i_task_manager*             m_task_manager;
    i_timer_manager*            m_timer_manager;
    net_engine_impl             m_net_engine;
    udp_transport_factory       m_socket_factory;

    inline static bool init(const net_core_config_t &config
                             , i_task_manager *task_manger
                             , i_timer_manager *timer_manager)
    {
        bool flag = false;
        if (m_init.compare_exchange_strong(flag
                                           , true))
        {
            m_single_core = create(config
                                   , task_manger
                                   , timer_manager);
            return true;
        }

        return false;
    }

    inline static bool cleanup()
    {
        bool flag = true;
        if (m_init.compare_exchange_strong(flag
                                           , false))
        {
            m_single_core.reset();
            return true;
        }

        return false;
    }

    inline static bool is_init()
    {
        return m_init.load(std::memory_order_acquire);
    }

    inline static net_core_impl& get_instance()
    {
        return *m_single_core;
    }

    inline static u_ptr_t create(const net_core_config_t &config
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
        , m_socket_factory(m_net_engine)
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

    inline i_timer_manager* select_timer_manager(i_timer_manager *timer_manager)
    {
        return timer_manager == nullptr
                ? m_timer_manager
                : timer_manager;

    }

    inline i_transport_factory* select_socket_factory(i_transport_factory *socket_factory)
    {
        return socket_factory == nullptr
                ? &m_socket_factory
                : socket_factory;

    }
};

net_core_impl::u_ptr_t net_core_impl::m_single_core = nullptr;
std::atomic_bool net_core_impl::m_init = false;


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

i_task_manager &net_core::task_manager()
{
    return *net_core_impl::get_instance().m_task_manager;
}

i_timer_manager &net_core::timer_manager()
{
    return *net_core_impl::get_instance().m_timer_manager;
}

i_transport_factory &net_core::socket_factory()
{
    return net_core_impl::get_instance().m_socket_factory;
}

i_transport_factory::u_ptr_t net_core::create_udp_factory()
{
    return udp_transport_factory::create(net_core_impl::get_instance().m_net_engine);
}

i_transport_factory::u_ptr_t net_core::create_ice_factory(const ice_config_t &ice_config
                                                          , i_transport_factory *socket_factory
                                                          , i_timer_manager *timer_manager)
{
    return ice_transport_factory::create(ice_config
                                         , *net_core_impl::get_instance().select_socket_factory(socket_factory)
                                         , *net_core_impl::get_instance().select_timer_manager(timer_manager));
}

i_transport_factory::u_ptr_t net_core::create_tls_factory(const tls_config_t &tls_config
                                                          , i_timer_manager* timer_manager)
{
    return tls_transport_factory::create(tls_config
                                         , *net_core_impl::get_instance().select_timer_manager(timer_manager));

}

i_net_packet::u_ptr_t net_core::create_packet(transport_id_t transport_id
                                              , const void *data
                                              , std::size_t size
                                              , bool copy
                                              , const socket_address_t &address)
{
    return detail::create_packet(transport_id
                                 , smart_buffer(data
                                                , size
                                                , copy)
                                 , address);
}

i_net_packet::u_ptr_t net_core::create_packet(transport_id_t transport_id
                                              , i_data_object::s_ptr_t shared_data
                                              , const socket_address_t &address)
{
    return detail::create_packet(transport_id
                                 , shared_data
                                 , address);
}



}
