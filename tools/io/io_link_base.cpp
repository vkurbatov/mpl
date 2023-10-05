#include "io_link_base.h"
#include "endpoint.h"

namespace io
{

io_link::io_link(io_core &core)
    : m_core(core)
    , m_message_handler(nullptr)
    , m_state_handler(nullptr)
    , m_state(link_state_t::ready)

{

}

void io_link::set_message_handler(const message_handler_t &message_handler)
{
    m_message_handler = message_handler;
}

void io_link::set_state_handler(const state_handler_t &state_handler)
{
    m_state_handler = state_handler;
}

bool io_link::set_endpoint(const endpoint_t &endpoint)
{
    return false;
}

link_state_t io_link::state() const
{
    return m_state;
}

io_core &io_link::core()
{
    return m_core;
}

bool io_link::send(const message_t &message)
{
    return send_to(message, endpoint_t::undefined());
}

void io_link::change_state(link_state_t new_state
                           , const std::string_view &reason)
{
    if (m_state != new_state)
    {
        m_state = new_state;
        on_change_state(new_state
                        , reason);
    }
}

void io_link::on_change_state(link_state_t new_state
                              , const std::string_view& reason)
{
    if (m_state_handler != nullptr)
    {
        m_state_handler(new_state
                        , reason);
    }
}

void io_link::on_recv_message(const message_t &message
                              , const endpoint_t &endpoint)
{
    if (m_message_handler != nullptr)
    {
        m_message_handler(message
                          , endpoint);
    }
}



}
