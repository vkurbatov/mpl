#ifndef MPL_NET_I_SERIAL_PACKET_H
#define MPL_NET_I_SERIAL_PACKET_H

#include "net/i_net_packet.h"
#include <string>

namespace mpl::net
{

class i_serial_packet : public i_net_packet
{
public:
    using u_ptr_t = std::unique_ptr<i_serial_packet>;
    using s_ptr_t = std::shared_ptr<i_serial_packet>;

    virtual std::string port_name() const = 0;
};

}


#endif // MPL_NET_I_SERIAL_PACKET_H