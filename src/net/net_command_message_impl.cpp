#include "utils/message_command_impl.tpp"

#include "net_module_types.h"
#include "net_command_types.h"
#include "ice/ice_gathering_command.h"

namespace mpl
{

template class message_command_impl<net::ice_gathering_command_t, net::net_module_id>;

}
