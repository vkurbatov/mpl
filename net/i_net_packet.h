#ifndef MPL_I_NET_PACKET_H
#define MPL_I_NET_PACKET_H

#include "net_types.h"
#include "core/i_message_packet.h"

namespace mpl::net
{

class i_net_packet : public i_message_packet
{
public:
    using u_ptr_t = std::unique_ptr<i_net_packet>;
    using s_ptr_t = std::shared_ptr<i_net_packet>;

    virtual transport_id_t transport_id() const = 0;
};

}

#endif // MPL_I_NET_PACKET_H
