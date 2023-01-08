#ifndef MPL_MESSAGE_SINK_IMPL_H
#define MPL_MESSAGE_SINK_IMPL_H

#include "i_message_sink.h"

#include <functional>

namespace mpl
{

class message_sink_impl : public i_message_sink
{
public:
    using message_handler_t = std::function<bool(const i_message& message)>;
private:
    message_handler_t   m_message_handler;
public:
    using u_ptr_t = std::unique_ptr<message_sink_impl>;
    using s_ptr_t = std::shared_ptr<message_sink_impl>;

    static u_ptr_t create(const message_handler_t& message_handler);

    message_sink_impl(const message_handler_t& message_handler);

    void set_handler(const message_handler_t& message_handler);

    // i_message_sink interface
public:
    bool send_message(const i_message &message) override;
};

}

#endif // MPL_MESSAGE_SINK_IMPL_H
