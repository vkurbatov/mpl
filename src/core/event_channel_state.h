#ifndef MPL_EVENT_CHANNEL_STATE_H
#define MPL_EVENT_CHANNEL_STATE_H

#include "channel_types.h"
#include "event.h"

namespace mpl
{

struct event_channel_state_t : public event_t
{
    constexpr static event_id_t id = core_channel_state_event_id;
    constexpr static std::string_view event_name = "channel_state";

    channel_state_t     state;
    std::string         reason;

    event_channel_state_t(channel_state_t state = channel_state_t::undefined
                          , const std::string_view& reason = {});

    bool operator == (const event_channel_state_t& other) const;


    // event_t interface
public:
    bool operator ==(const event_t &other) const override;
};

}

#endif // MPL_EVENT_CHANNEL_STATE_H
