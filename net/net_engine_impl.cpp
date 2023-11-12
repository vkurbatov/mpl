#include "net_engine_impl.h"

#include "utils/task_manager_impl.h"
#include "utils/timer_manager_impl.h"

#include "tools/io/io_core.h"
#include "tools/io/i_io_worker_factory.h"

#include "socket/socket_packet_builder_impl.h"
#include "serial/serial_packet_builder_impl.h"

#include "i_transport_collection.h"
#include "net_engine_config.h"
#include "net_module_types.h"

#include "socket/udp_transport_factory.h"
#include "ice/ice_transport_factory.h"
#include "tls/tls_transport_factory.h"

#include <list>
#include <atomic>
#include <thread>

namespace mpl::net
{

struct net_engine_impl final : public i_net_engine
        , i_net_module
        , i_transport_collection
        , pt::io::i_io_worker_factory
{
    using u_ptr_t = std::unique_ptr<net_engine_impl>;
    using promise_list_t = std::list<std::promise<void>>;

    net_engine_config_t     m_config;
    i_task_manager&         m_task_manager;
    i_timer_manager&        m_timer_manager;
    pt::io::io_core         m_io_core;
    promise_list_t          m_promises;

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

    void execute(worker_proc_t&& worker_proc
                 , std::promise<void>& promise)
    {
        auto executor = [worker_proc, &promise]
        {
            worker_proc();
            promise.set_value();
        };
        m_task_manager.add_task(executor);
    }

    ~net_engine_impl() override
    {
        net_engine_impl::stop();
    }

    bool start() override
    {
        bool flag  = false;
        if (m_start.compare_exchange_strong(flag, true))
        {
            if (m_task_manager.is_started()
                    && m_timer_manager.is_started())
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
            if (m_io_core.stop())
            {
                m_promises.clear();
                return true;
            }
        }


        return false;
    }

    bool is_started() const override
    {
        return m_io_core.is_running()
                && m_task_manager.is_started();
    }

    i_transport_collection& transports() override
    {
        return *this;
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

    // i_transport_collection interface
public:
    i_transport_factory *get_factory(transport_id_t transport_id) override
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

    // i_module interface
public:
    module_id_t module_id() const override
    {
       return net_module_id;
    }

    // i_net_engine interface
public:
    i_net_module &net() override
    {
        return *this;
    }

    // i_io_worker_factory interface
public:
    future_t execute_worker(worker_proc_t&& worker_proc) override
    {
        m_promises.emplace_back();
        auto future = m_promises.back().get_future();
        execute(std::move(worker_proc)
                , m_promises.back());
        return future;
    }
};

net_engine_factory::net_engine_factory(i_task_manager &task_manager
                                       , i_timer_manager &timer_manager)
    : m_task_manager(task_manager)
    , m_timer_manager(timer_manager)
{

}

i_net_engine::u_ptr_t net_engine_factory::create_engine(const net_engine_config_t &config)
{
    return net_engine_impl::create(config
                                   , m_task_manager
                                   , m_timer_manager);
}

}
