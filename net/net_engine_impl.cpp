#include "net_engine_impl.h"

#include "socket/udp_transport_factory.h"
#include "tools/io/io_core.h"

namespace mpl::net
{

net_engine_impl &net_engine_impl::get_instance()
{
    static net_engine_impl single_engine;
    return single_engine;
}

net_engine_impl::u_ptr_t net_engine_impl::create(task_manager_impl *task_manager)
{
    return std::make_unique<net_engine_impl>(task_manager);
}

net_engine_impl::net_engine_impl(task_manager_impl *task_manager)
    : m_task_manager(task_manager == nullptr ? &task_manager_impl::get_instance() : task_manager)
{

}


bool net_engine_impl::start()
{
    return m_task_manager->start();
}

bool net_engine_impl::stop()
{
    return m_task_manager->stop();
}

bool net_engine_impl::is_started() const
{
    return m_task_manager->is_started();
}

template<>
pt::io::io_core &net_engine_impl::get()
{
    return m_task_manager->get<pt::io::io_core>();
}

}
