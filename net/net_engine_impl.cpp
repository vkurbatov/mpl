#include "net_engine_impl.h"
#include "utils/task_manager_impl.h"
#include "tools/io/io_core.h"
#include "tools/io/i_io_worker_factory.h"

#include <atomic>

namespace mpl::net
{

struct net_engine_impl::pimpl_t
{
    using config_t = net_engine_impl::config_t;
    using u_ptr_t = net_engine_impl::pimpl_ptr_t;

    struct worker_factory final : public pt::io::i_io_worker_factory
    {
        i_task_manager&     m_task_manager;

        worker_factory(i_task_manager& task_manager)
            : m_task_manager(task_manager)
        {

        }
        // i_io_worker_factory interface
    public:
        bool execute_worker(const worker_proc_t &worker_proc) override
        {
            return m_task_manager.add_task(worker_proc) != nullptr;
        }
    };

    config_t            m_config;
    worker_factory      m_worker_factory;
    pt::io::io_core     m_io_core;

    std::atomic_bool    m_start;

    static u_ptr_t create(const config_t& config
                          , i_task_manager& task_manager)
    {
        return std::make_unique<pimpl_t>(config
                                         , task_manager);
    }

    pimpl_t(const config_t& config
            , i_task_manager& task_manager)
        : m_config(config)
        , m_worker_factory(task_manager)
        , m_io_core(m_config.max_workers
                    , &m_worker_factory)
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
            if (m_worker_factory.m_task_manager.is_started())
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
                && m_worker_factory.m_task_manager.is_started();
    }

};

net_engine_impl &net_engine_impl::get_instance()
{
    static net_engine_impl single_engine({}
                                         , task_manager_factory::single_manager());
    return single_engine;
}

net_engine_impl::u_ptr_t net_engine_impl::create(const config_t& config
                                                 , i_task_manager& task_manager)
{
    return std::make_unique<net_engine_impl>(config
                                             , task_manager);
}

net_engine_impl::net_engine_impl(const config_t& config
                                 , i_task_manager& task_manager)
    : m_pimpl(pimpl_t::create(config
                              , task_manager))
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

}
