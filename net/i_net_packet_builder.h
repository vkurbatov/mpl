#ifndef MPL_I_NET_PACKET_BUILDER_H
#define MPL_I_NET_PACKET_BUILDER_H

#include "i_net_packet.h"
#include "socket/socket_types.h"

namespace mpl::net
{

class i_net_packet_builder
{
public:
    using u_ptr_t = std::unique_ptr<i_net_packet_builder>;
    using s_ptr_t = std::shared_ptr<i_net_packet_builder>;

    virtual ~i_net_packet_builder() = default;

    virtual transport_id_t transport_id() const = 0;

    virtual void set_packet_data(const void* packet_data
                                 , std::size_t packet_size) = 0;

    virtual const i_data_object& packet_data() const = 0;

    virtual i_net_packet::u_ptr_t build_packet() = 0;

};

}

#endif // MPL_I_NET_PACKET_BUILDER_H
