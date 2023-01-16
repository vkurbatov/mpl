#include "message_router_impl.h"
#include <shared_mutex>

namespace mpl
{

namespace
{

template<typename T>
using lock_t = std::lock_guard<T>;

template<typename T>
using shared_lock_t = std::shared_lock<T>;

}

message_router_impl::u_ptr_t message_router_impl::create(i_message_sink *sink)
{
    return std::make_unique<message_router_impl>(sink);
}

message_router_impl::message_router_impl(i_message_sink *sink)
{
    message_router_impl::add_sink(sink);
}

message_router_impl::~message_router_impl()
{
    lock_t lock(m_safe_mutex);
    m_sinks.clear();
}

bool message_router_impl::add_sink(i_message_sink *sink)
{
    if (sink)
    {
        lock_t lock(m_safe_mutex);
        return m_sinks.emplace(sink).second;
    }

    return false;
}

bool message_router_impl::remove_sink(i_message_sink *sink)
{
    if (sink)
    {
        lock_t lock(m_safe_mutex);
        return m_sinks.erase(sink) > 0;
    }

    return false;
}

bool message_router_impl::send_message(const i_message &message)
{
    bool result = false;
    shared_lock_t lock(m_safe_mutex);

    for (auto s : m_sinks)
    {
        result |= s->send_message(message);
    }

    return result;
}



}

