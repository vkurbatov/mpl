#ifndef MPL_MESSAGE_SINK_IMPL_H
#define MPL_MESSAGE_SINK_IMPL_H

#include "core/i_message_sink.h"

#include <functional>
#include "tools/base/sync_base.h"

namespace mpl
{


class message_sink_impl : public i_message_sink
{
public:
    using message_handler_t = std::function<bool(const i_message& message)>;
private:
    message_handler_t       m_message_handler;
public:
    using u_ptr_t = std::unique_ptr<message_sink_impl>;
    using s_ptr_t = std::shared_ptr<message_sink_impl>;

    static message_handler_t create_handler(i_message_sink* message_sink);
    static u_ptr_t create(const message_handler_t& message_handler);
    static u_ptr_t create(i_message_sink* message_sink);

    message_sink_impl(const message_handler_t& message_handler);
    message_sink_impl(i_message_sink* message_sink);

    void set_handler(const message_handler_t& message_handler);
    void set_message_sink(i_message_sink* message_sink);

    // i_message_sink interface
public:
    bool send_message(const i_message &message) override;
};

class message_sink_safe_impl : public message_sink_impl
{
    mutable base::spin_lock m_safe_mutex;
public:
    using u_ptr_t = std::unique_ptr<message_sink_impl>;
    using s_ptr_t = std::shared_ptr<message_sink_impl>;

    static u_ptr_t create(const message_handler_t& message_handler);
    static u_ptr_t create(i_message_sink* message_sink);

    message_sink_safe_impl(const message_handler_t& message_handler);
    message_sink_safe_impl(i_message_sink* message_sink);
    // i_message_sink interface
public:
    bool send_message(const i_message &message) override;
};

}

#endif // MPL_MESSAGE_SINK_IMPL_H
