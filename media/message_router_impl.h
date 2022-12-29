#ifndef MPL_MESSAGE_ROUTER_IMPL_H
#define MPL_MESSAGE_ROUTER_IMPL_H

#include "i_message_sink.h"
#include "i_message_source.h"

#include "tools/base/sync_base.h"

#include <unordered_set>

namespace mpl
{

class message_router_impl : public i_message_sink
        , public i_message_source
{
    using mutex_t = base::shared_spin_lock;
    using sink_set_t = std::unordered_set<i_message_sink*>;
    mutable mutex_t     m_safe_mutex;
    sink_set_t          m_sinks;

public:

    using u_ptr_t = std::unique_ptr<message_router_impl>;
    using s_ptr_t = std::unique_ptr<message_router_impl>;

    static u_ptr_t create(i_message_sink* sink = nullptr);

    message_router_impl(i_message_sink* sink = nullptr);
    ~message_router_impl();

    // i_message_source interface
public:
    bool add_sink(i_message_sink *sink) override;
    bool remove_sink(i_message_sink *sink) override;

    // i_message_sink interface
public:
    bool send_message(const i_message &message) override;
};

}

#endif // MPL_MESSAGE_ROUTER_IMPL_H
