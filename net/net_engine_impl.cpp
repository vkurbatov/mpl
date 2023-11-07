#include "net_engine_impl.h"

#include "utils/task_manager_impl.h"
#include "utils/timer_manager_impl.h"

#include "tools/io/io_core.h"
#include "tools/io/i_io_worker_factory.h"

#include "socket/socket_packet_builder_impl.h"
#include "serial/serial_packet_builder_impl.h"

#include "net_engine_config.h"

#include "socket/udp_transport_factory.h"
#include "ice/ice_transport_factory.h"
#include "tls/tls_transport_factory.h"


#include <atomic>

namespace mpl::net
{

struct net_engine_impl final : public pt::io::i_io_worker_factory
        , public i_net_engine
{
    using u_ptr_t = std::unique_ptr<net_engine_impl>;

    net_engine_config_t     m_config;
    i_task_manager&         m_task_manager;
    i_timer_manager&        m_timer_manager;
    pt::io::io_core         m_io_core;

    udp_transport_factory   m_udp_factory;
    ice_transport_factory   m_ice_factory;
    tls_transport_factory   m_tls_factory;

    std::atomic_bool        m_start;

    static u_ptr_t create(const net_engine_config_t& config
                          , i_task_manager& task_manager
                          , i_timer_manager& timer_manager)
    {
        return std::make_unique<net_engine_impl>(config
                                                , task_manager
                                                , timer_manager);
    }

    net_engine_impl(const net_engine_config_t& config
                    , i_task_manager& task_manager
                    , i_timer_manager& timer_manager)
        : m_config(config)
        , m_task_manager(task_manager)
        , m_timer_manager(timer_manager)
        , m_io_core(m_config.listen_workers
                    , this)
        , m_udp_factory(m_io_core)
        , m_ice_factory(m_config.ice_config
                        , m_udp_factory
                        , m_timer_manager)
        , m_tls_factory(m_config.tls_config
                        , m_timer_manager)
        , m_start(false)
    {

    }

    ~net_engine_impl() override
    {
        net_engine_impl::stop();
    }

    // i_net_engine interface
public:
    i_task_manager &task_manager() override
    {
        return m_task_manager;
    }

    i_timer_manager &timer_manager() override
    {
        return m_timer_manager;
    }

    bool start() override
    {
        bool flag  = false;
        if (m_start.compare_exchange_strong(flag, true))
        {
            if (m_task_manager.is_started())
            {
                if (m_io_core.run())
                {
                    return true;
                }
            }

            m_start.store(false, std::memory_order_release);
        }

        return false;
    }

    inline bool stop() override
    {
        bool flag = true;
        if (m_start.compare_exchange_strong(flag, false))
        {
            return m_io_core.stop();
        }

        return false;
    }

    bool is_started() const override
    {
        return m_io_core.is_running()
                && m_task_manager.is_started();
    }

    i_transport_factory* transport_factory(transport_id_t transport_id) override
    {
        switch(transport_id)
        {
            case transport_id_t::udp:
                return &m_udp_factory;
            break;
            case transport_id_t::tcp:

            break;
            case transport_id_t::ice:
                return &m_ice_factory;
            break;
            case transport_id_t::tls:
                return &m_tls_factory;
            break;
            default:;
        }

        return nullptr;
    }

    i_net_packet_builder::u_ptr_t create_packet_builder(transport_id_t transport_id) override
    {
        switch(transport_id)
        {
            case transport_id_t::udp:
            case transport_id_t::tcp:
            case transport_id_t::ice:
            case transport_id_t::tls:
                return socket_packet_builder_impl::create(transport_id);
            break;
            case transport_id_t::serial:
                return serial_packet_builder_impl::create();
            break;
            default:;
        }
        return nullptr;
    }

    // i_io_worker_factory interface
public:
    bool execute_worker(const worker_proc_t &worker_proc) override
    {
        return m_task_manager.add_task(worker_proc) != nullptr;
    }

};

net_engine_factory &net_engine_factory::get_instance()
{
    static net_engine_factory single_factory;
    return single_factory;
}

i_net_engine::u_ptr_t net_engine_factory::create_engine(const net_engine_config_t &config
                                                        , i_task_manager &task_manager
                                                        , i_timer_manager &timer_manager)
{
    return net_engine_impl::create(config
                                   , task_manager
                                   , timer_manager);
}

}
