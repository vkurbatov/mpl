#include "net_engine_impl.h"

#include "utils/task_manager_impl.h"
#include "utils/timer_manager_impl.h"

#include "tools/io/io_core.h"
#include "tools/io/i_io_worker_factory.h"

#include "net_packet_builder_impl.h"
#include "net_engine_config.h"

#include "socket/udp_transport_factory.h"
#include "ice/ice_transport_factory.h"
#include "tls/tls_transport_factory.h"


#include <atomic>

namespace mpl::net
{

struct net_engine_impl::pimpl_t : public pt::io::i_io_worker_factory
{
    using u_ptr_t = net_engine_impl::pimpl_ptr_t;

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
        return std::make_unique<pimpl_t>(config
                                         , task_manager
                                         , timer_manager);
    }

    pimpl_t(const net_engine_config_t& config
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

    ~pimpl_t()
    {
        stop();
    }

    inline bool start()
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

    inline bool stop()
    {
        bool flag = true;
        if (m_start.compare_exchange_strong(flag, false))
        {
            return m_io_core.stop();
        }

        return false;
    }

    inline bool is_started() const
    {
        return m_io_core.is_running()
                && m_task_manager.is_started();
    }

    inline i_transport_factory* transport_factory(transport_id_t transport_id)
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


    // i_io_worker_factory interface
public:
    bool execute_worker(const worker_proc_t &worker_proc) override
    {
        return m_task_manager.add_task(worker_proc) != nullptr;
    }

};

struct net_engine_config_storage_t
{
    static net_engine_config_t  net_engine_config;
};

void net_engine_impl::set_default_config(const net_engine_config_t &config)
{
    net_engine_config_storage_t::net_engine_config = config;
}

const net_engine_config_t &net_engine_impl::default_config()
{
    return net_engine_config_storage_t::net_engine_config;
}

net_engine_impl &net_engine_impl::get_instance()
{
    static net_engine_impl single_engine(net_engine_config_storage_t::net_engine_config
                                         , task_manager_factory::single_manager()
                                         , timer_manager_factory::single_manager());
    return single_engine;
}

net_engine_impl::u_ptr_t net_engine_impl::create(const net_engine_config_t& config
                                                 , i_task_manager& task_manager
                                                 , i_timer_manager& timer_manager)
{
    return std::make_unique<net_engine_impl>(config
                                             , task_manager
                                             , timer_manager);
}

net_engine_impl::net_engine_impl(const net_engine_config_t& config
                                 , i_task_manager& task_manager
                                 , i_timer_manager& timer_manager)
    : m_pimpl(pimpl_t::create(config
                              , task_manager
                              , timer_manager))
{

}

net_engine_impl::~net_engine_impl()
{

}

pt::io::io_core &net_engine_impl::io_core()
{
    return m_pimpl->m_io_core;
}

bool net_engine_impl::start()
{
    return m_pimpl->start();
}

bool net_engine_impl::stop()
{
    return m_pimpl->stop();
}

bool net_engine_impl::is_started() const
{
    return m_pimpl->is_started();
}

i_task_manager &net_engine_impl::task_manager()
{
    return m_pimpl->m_task_manager;
}

i_timer_manager &net_engine_impl::timer_manager()
{
    return m_pimpl->m_timer_manager;
}

i_transport_factory *net_engine_impl::transport_factory(transport_id_t transport_id)
{
    return m_pimpl->transport_factory(transport_id);
}

i_net_packet_builder::u_ptr_t net_engine_impl::create_packet_builder(transport_id_t transport_id)
{
    switch(transport_id)
    {
        case transport_id_t::udp:
        case transport_id_t::tcp:
        case transport_id_t::ice:
        case transport_id_t::tls:
            return net_packet_builder_impl::create(transport_id);
        break;
    }

    return nullptr;
}

}
