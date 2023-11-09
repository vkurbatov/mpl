#include "message_event_impl.h"

namespace mpl
{

template<typename Event, module_id_t ModuleId>
typename message_event_impl<Event, ModuleId>::u_ptr_t message_event_impl<Event, ModuleId>::create(const Event &event)
{
    return std::make_unique<message_event_impl>(event);
}

template<typename Event, module_id_t ModuleId>
typename message_event_impl<Event, ModuleId>::u_ptr_t message_event_impl<Event, ModuleId>::create(Event &&event)
{
    return std::make_unique<message_event_impl>(std::move(event));
}

template<typename Event, module_id_t ModuleId>
message_event_impl<Event, ModuleId>::message_event_impl(const Event &event)
    : m_event(event)
{

}

template<typename Event, module_id_t ModuleId>
message_event_impl<Event, ModuleId>::message_event_impl(Event &&event)
    : m_event(std::move(event))
{

}

template<typename Event, module_id_t ModuleId>
void message_event_impl<Event, ModuleId>::set_event(const Event &event)
{
    m_event = event;
}

template<typename Event, module_id_t ModuleId>
void message_event_impl<Event, ModuleId>::set_event(Event &&event)
{
    m_event = std::move(event);
}

template<typename Event, module_id_t ModuleId>
message_category_t message_event_impl<Event, ModuleId>::category() const
{
    return message_category_t::event;
}

template<typename Event, module_id_t ModuleId>
module_id_t message_event_impl<Event, ModuleId>::module_id() const
{
    return ModuleId;
}

template<typename Event, module_id_t ModuleId>
i_message::u_ptr_t message_event_impl<Event, ModuleId>::clone() const
{
    return create(m_event);
}

template<typename Event, module_id_t ModuleId>
const event_t &message_event_impl<Event, ModuleId>::event() const
{
    return m_event;
}

}
