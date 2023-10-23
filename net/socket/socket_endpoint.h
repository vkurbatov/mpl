#ifndef MPL_NET_SOCKET_ENDPOINT_H
#define MPL_NET_SOCKET_ENDPOINT_H

#include "socket_types.h"
#include "net/endpoint.h"

namespace mpl::net
{

template<transport_id_t Transport>
struct socket_endpoint_t : public endpoint_t
{
    using array_t = std::vector<socket_endpoint_t>;

    socket_address_t       socket_address;

    socket_endpoint_t(const socket_address_t& socket_address = {});

    std::size_t hash() const;


    // endpoint_t interface
public:
    bool operator ==(const endpoint_t &other) const override;
    bool is_valid() const override;
};

using udp_endpoint_t = socket_endpoint_t<transport_id_t::udp>;
using tcp_endpoint_t = socket_endpoint_t<transport_id_t::tcp>;

}

#endif // MPL_NET_SOCKET_ENDPOINT_H
