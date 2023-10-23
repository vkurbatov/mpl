#include "net_engine_impl.h"

#include "socket/udp_transport_factory.h"

namespace mpl::net
{

net_engine_impl &net_engine_impl::get_instance()
{
    static net_engine_impl single_engine;
    return single_engine;
}

net_engine_impl::u_ptr_t net_engine_impl::create(std::size_t max_workers)
{
    return std::make_unique<net_engine_impl>(max_workers);
}

net_engine_impl::net_engine_impl(std::size_t max_workers)
    : m_io_core(max_workers)
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
