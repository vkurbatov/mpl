#include "message_event_impl.h"

namespace mpl
{

template<typename Event, message_subclass_t Subclass>
typename message_event_impl<Event, Subclass>::u_ptr_t message_event_impl<Event, Subclass>::create(const Event &event)
{
    return std::make_unique<message_event_impl>(event);
}

template<typename Event, message_subclass_t Subclass>
typename message_event_impl<Event, Subclass>::u_ptr_t message_event_impl<Event, Subclass>::create(Event &&event)
{
    return std::make_unique<message_event_impl>(std::move(event));
}

template<typename Event, message_subclass_t Subclass>
message_event_impl<Event, Subclass>::message_event_impl(const Event &event)
    : m_event(event)
{

}

template<typename Event, message_subclass_t Subclass>
message_event_impl<Event, Subclass>::message_event_impl(Event &&event)
    : m_event(std::move(event))
{

}

template<typename Event, message_subclass_t Subclass>
void message_event_impl<Event, Subclass>::set_event(const Event &event)
{
    m_event = event;
}

template<typename Event, message_subclass_t Subclass>
void message_event_impl<Event, Subclass>::set_event(Event &&event)
{
    m_event = std::move(event);
}

template<typename Event, message_subclass_t Subclass>
message_category_t message_event_impl<Event, Subclass>::category() const
{
    return message_category_t::event;
}

template<typename Event, message_subclass_t Subclass>
message_subclass_t message_event_impl<Event, Subclass>::subclass() const
{
    return Subclass;
}

template<typename Event, message_subclass_t Subclass>
i_message::u_ptr_t message_event_impl<Event, Subclass>::clone() const
{
    return create(m_event);
}

template<typename Event, message_subclass_t Subclass>
const event_t &message_event_impl<Event, Subclass>::event() const
{
    return m_event;
}

}
