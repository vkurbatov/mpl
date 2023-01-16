#ifndef MPL_EVENT_CHANNEL_STATE_H
#define MPL_EVENT_CHANNEL_STATE_H

#include "channel_types.h"
#include "event.h"

namespace mpl
{

struct event_channel_state_t : public event_t
{
    channel_state_t     state;
    std::string         reason;

    event_channel_state_t(channel_state_t state = channel_state_t::undefined
                          , const std::string_view& reason = {});

};

}

#endif // MPL_EVENT_CHANNEL_STATE_H
