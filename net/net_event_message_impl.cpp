#include "utils/message_event_impl.tpp"
#include "net_message_types.h"
#include "net_event_types.h"
#include "ice/ice_gathering_state_event.h"

namespace mpl
{

template class message_event_impl<net::ice_gathering_state_event_t, net::message_net_class>;

}
