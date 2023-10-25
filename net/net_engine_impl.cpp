#include "net_engine_impl.h"
#include "utils/task_manager_impl.h"
#include "tools/io/io_core.h"

namespace mpl::net
{

net_engine_impl::worker_factory::worker_factory(i_task_manager &task_manager)
    : m_task_manager(task_manager)
{

}

bool net_engine_impl::worker_factory::execute_worker(const worker_proc_t &worker_proc)
{
    return m_task_manager.add_task(worker_proc) != nullptr;
}


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
    : m_worker_factory(task_manager)
    , m_io_core({config.max_workers}
                , &m_worker_factory)
{

}

pt::io::io_core &net_engine_impl::io_core()
{
    return m_io_core;
}


bool net_engine_impl::start()
{
    return m_io_core.run();
}

bool net_engine_impl::stop()
{
    return m_io_core.stop();
}

bool net_engine_impl::is_started() const
{
    return m_io_core.is_running();
}

}
