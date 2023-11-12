#include "event_channel_state.h"


namespace mpl
{

// event_t::event_id_t event_channel_state_t::id = event_t::register_event(event_channel_state_t::event_name);

event_channel_state_t::event_channel_state_t(channel_state_t state
                                             , const std::string_view &reason)
    : event_t(id
              , event_name)
    , state(state)
    , reason(reason)
{

}

bool event_channel_state_t::operator ==(const event_channel_state_t &other) const
{
    return state == other.state
            && reason == other.reason;
}

bool event_channel_state_t::operator ==(const event_t &other) const
{
    return event_t::operator == (other)
            && *this == (static_cast<const event_channel_state_t&>(other));
}

}
