#include "message_event_impl.h"

#include "core/event_channel_state.h"

namespace mpl
{

template class message_event_impl<event_channel_state_t>;

template<typename Event>
typename message_event_impl<Event>::u_ptr_t message_event_impl<Event>::create(const Event &event)
{
    return std::make_unique<message_event_impl>(event);
}

template<typename Event>
typename message_event_impl<Event>::u_ptr_t message_event_impl<Event>::create(Event &&event)
{
    return std::make_unique<message_event_impl>(std::move(event));
}

template<typename Event>
message_event_impl<Event>::message_event_impl(const Event &event)
    : m_event(event)
{

}

template<typename Event>
message_event_impl<Event>::message_event_impl(Event &&event)
    : m_event(std::move(event))
{

}

template<typename Event>
void message_event_impl<Event>::set_event(const Event &event)
{
    m_event = event;
}

template<typename Event>
void message_event_impl<Event>::set_event(Event &&event)
{
    m_event = std::move(event);
}

template<typename Event>
message_category_t message_event_impl<Event>::category() const
{
    return message_category_t::event;
}

template<typename Event>
message_subtype_t message_event_impl<Event>::subtype() const
{
    return static_cast<message_subtype_t>(m_event.event_id);
}

template<typename Event>
i_message::u_ptr_t message_event_impl<Event>::clone() const
{
    return create(m_event);
}

template<typename Event>
const event_t &message_event_impl<Event>::event() const
{
    return m_event;
}



}
