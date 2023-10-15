#include "net_utils.h"
#include "tools/io/io_base.h"

namespace mpl::utils
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

net::protocol_type_t parse_protocol(const void *data, std::size_t size)
{
    if (size > 0
            && data != nullptr)
    {
        const auto ptr = static_cast<const std::uint8_t*>(data);
        const auto& signature = *ptr;
        if (signature > 127
                && signature < 192)
        {
            if (size >= 8)
            {
                const auto& pt = ptr[1];
                if (pt >= 200 && pt <= 223)
                {
                    return net::protocol_type_t::rtcp;
                }
                else if (size >= 12)
                {
                    return net::protocol_type_t::rtp;
                }
            }
        }
        else if ((signature & 0xfe) == 0)
        {
            if (size >= 20)
            {
                return net::protocol_type_t::stun;
            }
        }
        else if (signature > 19
                  && signature < 24)
        {
            if (size >= 10
                    && ptr[1] == 0xfe)
            {
                return net::protocol_type_t::tls;
            }
        }
    }

    return net::protocol_type_t::undefined;
}



}
