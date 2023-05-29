#include "message_sink_impl.h"
#include <mutex>

namespace mpl
{

message_sink_impl::message_handler_t message_sink_impl::create_handler(i_message_sink *message_sink)
{
    if (message_sink)
    {
        return [message_sink](const i_message& message) { return message_sink->send_message(message); };
    }

    return nullptr;
}

message_sink_impl::u_ptr_t message_sink_impl::create(const message_handler_t &message_handler)
{
    return std::make_unique<message_sink_impl>(message_handler);
}


message_sink_impl::u_ptr_t message_sink_impl::create(i_message_sink *message_sink)
{
    return std::make_unique<message_sink_impl>(message_sink);
}

message_sink_impl::message_sink_impl(const message_handler_t &message_handler)
    : m_message_handler(message_handler)
{

}

message_sink_impl::message_sink_impl(i_message_sink *message_sink)
    : message_sink_impl(create_handler(message_sink))
{

}

void message_sink_impl::set_handler(const message_handler_t &message_handler)
{
    m_message_handler = message_handler;
}

void message_sink_impl::set_message_sink(i_message_sink *message_sink)
{
    m_message_handler = create_handler(message_sink);
}

bool message_sink_impl::send_message(const i_message &message)
{
    if (auto h = m_message_handler)
    {
        return h(message);
    }

    return false;
}

message_sink_safe_impl::u_ptr_t message_sink_safe_impl::create(const message_handler_t &message_handler)
{
    return std::make_unique<message_sink_safe_impl>(message_handler);
}

message_sink_safe_impl::u_ptr_t message_sink_safe_impl::create(i_message_sink *message_sink)
{
    return std::make_unique<message_sink_safe_impl>(message_sink);
}

message_sink_safe_impl::message_sink_safe_impl(const message_handler_t &message_handler)
    : message_sink_impl(message_handler)
{

}

message_sink_safe_impl::message_sink_safe_impl(i_message_sink *message_sink)
    : message_sink_impl(message_sink)
{

}

bool message_sink_safe_impl::send_message(const i_message &message)
{
    std::lock_guard<base::spin_lock> lock(m_safe_mutex);
    return message_sink_impl::send_message(message);
}

}
