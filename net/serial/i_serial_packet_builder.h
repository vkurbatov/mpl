#ifndef MPL_NET_I_SERIAL_PACKET_BUILDER_H
#define MPL_NET_I_SERIAL_PACKET_BUILDER_H

#include "net/i_net_packet_builder.h"
#include "i_serial_packet.h"

namespace mpl::net
{

class i_serial_packet_builder : public i_net_packet_builder
{
public:
    using u_ptr_t = std::unique_ptr<i_serial_packet_builder>;
    using s_ptr_t = std::shared_ptr<i_serial_packet_builder>;

    virtual void set_port_name(const std::string_view& port_name) = 0;
    virtual std::string port_name() const = 0;
};

}

#endif // MPL_NET_I_SERIAL_PACKET_BUILDER_H
