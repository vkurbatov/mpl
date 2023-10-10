#include "net_utils.h"
#include "tools/io/io_base.h"

namespace mpl::net
{

template<>
channel_state_t get_channel_state(const io::link_state_t &other_state)
{
    switch(other_state)
    {
        case io::link_state_t::ready:
            return channel_state_t::ready;
        break;
        case io::link_state_t::opening:
            return channel_state_t::opening;
        break;
        case io::link_state_t::open:
            return channel_state_t::open;
        break;
        case io::link_state_t::connecting:
            return channel_state_t::connecting;
        break;
        case io::link_state_t::connected:
            return channel_state_t::connected;
        break;
        case io::link_state_t::disconnecting:
            return channel_state_t::disconnecting;
        break;
        case io::link_state_t::disconnected:
            return channel_state_t::disconnected;
        break;
        case io::link_state_t::closing:
            return channel_state_t::closing;
        break;
        case io::link_state_t::closed:
            return channel_state_t::closed;
        break;
        case io::link_state_t::failed:
            return channel_state_t::failed;
        break;
        default:;
    }

    return channel_state_t::undefined;
}



}
