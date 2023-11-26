#ifndef MPL_NET_ICE_GATHERING_STATE_EVENT_H
#define MPL_NET_ICE_GATHERING_STATE_EVENT_H

#include "core/event.h"
#include "net/net_event_types.h"
#include "ice_types.h"

namespace mpl::net
{

struct ice_gathering_state_event_t : public event_t
{
    constexpr static event_id_t id = net_ice_gathering_state_event_id;
    constexpr static std::string_view event_name = "ice_gathering_state";

    ice_gathering_state_t   state;
    std::string             reason;

    ice_gathering_state_event_t(ice_gathering_state_t state = ice_gathering_state_t::undefined
                                , const std::string_view& reason = {});

    bool operator == (const ice_gathering_state_event_t& other) const;


    // event_t interface
public:
    bool operator ==(const event_t &other) const override;
};

}

#endif // MPL_NETICE_GATHERING_STATE_EVENT_H
