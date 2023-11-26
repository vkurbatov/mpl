#include "socket_endpoint.h"

template<>
struct std::hash<mpl::net::udp_endpoint_t>
{
    std::size_t operator()(const mpl::net::udp_endpoint_t& s) const noexcept
    {
        return s.hash();
    }
};

template<>
struct std::hash<mpl::net::tcp_endpoint_t>
{
    std::size_t operator()(const mpl::net::tcp_endpoint_t& s) const noexcept
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

template struct socket_endpoint_t<transport_id_t::udp>;
template struct socket_endpoint_t<transport_id_t::tcp>;

template<transport_id_t Transport>
socket_endpoint_t<Transport>::socket_endpoint_t(const socket_address_t &socket_address)
    : endpoint_t(Transport)
    , socket_address(socket_address)
{

}

template<transport_id_t Transport>
std::size_t socket_endpoint_t<Transport>::hash() const
{
    return socket_address.hash()
            ^ std::hash<std::size_t>()(static_cast<std::size_t>(transport_id));
}

template<transport_id_t Transport>
bool socket_endpoint_t<Transport>::operator ==(const endpoint_t &other) const
{
    return transport_id == other.transport_id
            && socket_address == static_cast<const socket_endpoint_t&>(other).socket_address;
}

template<transport_id_t Transport>
bool socket_endpoint_t<Transport>::is_valid() const
{
    return (transport_id == Transport)
            && socket_address.is_valid();
}

}
