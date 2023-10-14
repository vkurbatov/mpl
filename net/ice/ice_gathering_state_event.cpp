#include "ice_gathering_state_event.h"

namespace mpl::net
{

ice_gathering_state_event_t::ice_gathering_state_event_t(ice_gathering_state_t state
                                                        , const std::string_view &reason)
    : event_t(id
              , event_name)
    , state(state)
    , reason(reason)
{

}

bool ice_gathering_state_event_t::operator ==(const ice_gathering_state_event_t &other) const
{
    return state == other.state
            && reason == other.reason;
}

bool ice_gathering_state_event_t::operator ==(const event_t &other) const
{
    return event_t::operator == (other)
            && *this == (static_cast<const ice_gathering_state_event_t&>(other));
}


}
