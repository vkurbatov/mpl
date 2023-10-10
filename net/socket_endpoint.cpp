#include "socket_endpoint.h"

namespace mpl::net
{

namespace detail
{

inline transport_id_t transport_from_socket_type(socket_type_t socket_type)
{
    return socket_type == socket_type_t::udp
            ? transport_id_t::udp
            : transport_id_t::tcp;
}

}

socket_endpoint_t::socket_endpoint_t(socket_type_t socket_type
                                     , const ip_endpoint_t &ip_endpoint)
    : endpoint_t(detail::transport_from_socket_type(socket_type))
    , ip_endpoint(ip_endpoint)
{

}

bool socket_endpoint_t::operator ==(const endpoint_t &other) const
{
    return transport_id == other.transport_id
            && ip_endpoint == static_cast<const socket_endpoint_t&>(other).ip_endpoint;
}

bool socket_endpoint_t::is_valid() const
{
    return (transport_id == transport_id_t::tcp
            || transport_id == transport_id_t::udp)
            && ip_endpoint.is_valid();
}



}
