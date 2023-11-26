#ifndef IO_LINK_H
#define IO_LINK_H

#include "i_io_link.h"

namespace pt::io
{

class io_core;

class io_link : public i_io_link
{

protected:
    io_core&            m_core;

    message_handler_t   m_message_handler;
    state_handler_t     m_state_handler;

    link_state_t        m_state;

public:

    using u_ptr_t = std::unique_ptr<io_link>;

    io_link(io_core& core);

    void set_message_handler(const message_handler_t& message_handler) final override;
    void set_state_handler(const state_handler_t& state_handler) final override;
    bool set_endpoint(const endpoint_t &endpoint) override;
    link_state_t state() const final override;

    io_core& core();

    bool send(const message_t& message);
protected:
    void change_state(link_state_t new_state
                      , const std::string_view& reason = {});
    void on_change_state(link_state_t new_state
                         , const std::string_view& reason);
    void on_recv_message(const message_t& message
                         , const endpoint_t& endpoint);
};

}

#endif // IO_LINK_BASE_H
