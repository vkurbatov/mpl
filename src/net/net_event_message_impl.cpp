#include "utils/message_event_impl.tpp"
#include "net_module_types.h"
#include "net_event_types.h"
#include "ice/ice_gathering_state_event.h"
#include "tls/tls_keys_event.h"

namespace mpl
{

template class message_event_impl<net::ice_gathering_state_event_t, net::net_module_id>;
template class message_event_impl<net::tls_keys_event_t, net::net_module_id>;


}
