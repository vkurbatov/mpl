#ifndef MPL_NET_I_SERIAL_TRANSPORT_H
#define MPL_NET_I_SERIAL_TRANSPORT_H

#include "net/i_transport_channel.h"
#include "serial_endpoint.h"

namespace mpl::net
{

class i_serial_transport : public i_transport_channel
{
public:
    using u_ptr_t = std::unique_ptr<i_serial_transport>;
    using s_ptr_t = std::shared_ptr<i_serial_transport>;

    virtual bool set_endpoint(const serial_endpoint_t& serial_endpoint) = 0;
    virtual serial_endpoint_t endpoint() const = 0;
};

}

#endif // MPL_NET_I_SERIAL_TRANSPORT_H
