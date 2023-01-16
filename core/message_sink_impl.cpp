#include "message_sink_impl.h"

namespace mpl
{

message_sink_impl::u_ptr_t message_sink_impl::create(const message_handler_t &message_handler)
{
    return std::make_unique<message_sink_impl>(message_handler);
}

message_sink_impl::message_sink_impl(const message_handler_t &message_handler)
    : m_message_handler(message_handler)
{

}

void message_sink_impl::set_handler(const message_handler_t &message_handler)
{
    m_message_handler = message_handler;
}

bool message_sink_impl::send_message(const i_message &message)
{
    if (auto h = m_message_handler)
    {
        return h(message);
    }

    return false;
}

}
