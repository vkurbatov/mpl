#include "socket_endpoint.h"

template<>
struct std::hash<mpl::net::socket_endpoint_t>
{
    std::size_t operator()(const mpl::net::socket_endpoint_t& s) const noexcept
    {
        return s.hash();
    }
};

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
                                     , const socket_address_t &ip_endpoint)
    : endpoint_t(detail::transport_from_socket_type(socket_type))
    , socket_address(ip_endpoint)
{

}

std::size_t socket_endpoint_t::hash() const
{
    return socket_address.hash()
            ^ std::hash<std::size_t>()(static_cast<std::size_t>(transport_id));
}

bool socket_endpoint_t::operator ==(const endpoint_t &other) const
{
    return transport_id == other.transport_id
            && socket_address == static_cast<const socket_endpoint_t&>(other).socket_address;
}

bool socket_endpoint_t::is_valid() const
{
    return (transport_id == transport_id_t::tcp
            || transport_id == transport_id_t::udp)
            && socket_address.is_valid();
}

}
