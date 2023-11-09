#ifndef MPL_MESSAGE_EVENT_IMPL_H
#define MPL_MESSAGE_EVENT_IMPL_H

#include "core/i_message_event.h"

namespace mpl
{

template<typename Event, module_id_t ModuleId = core_module_id>
class message_event_impl : public i_message_event
{
    Event   m_event;
public:
    using u_ptr_t = std::unique_ptr<message_event_impl>;
    using s_ptr_t = std::shared_ptr<message_event_impl>;

    static u_ptr_t create(const Event& event);
    static u_ptr_t create(Event&& event);

    message_event_impl(const Event& event);
    message_event_impl(Event&& event);

    void set_event(const Event& event);
    void set_event(Event&& event);

    // i_message interface
public:
    message_category_t category() const override;
    module_id_t module_id() const override;
    i_message::u_ptr_t clone() const override;

    // i_message_event interface
public:
    const event_t &event() const override;
};

}

#endif // MPL_MESSAGE_EVENT_IMPL_H
