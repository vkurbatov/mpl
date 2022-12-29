#include "event_channel_state.h"


namespace mpl
{

event_channel_state_t::event_channel_state_t(channel_state_t state
                                             , const std::string_view &reason)
    : event_t(event_id_t::channel_state
              , "channel_state")
    , state(state)
    , reason(reason)
{

}

}
