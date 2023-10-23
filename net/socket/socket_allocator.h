#ifndef MPL_NET_SOCKET_ALLOCATOR_H
#define MPL_NET_SOCKET_ALLOCATOR_H

#include "net/i_transport_factory.h"
#include "net/socket/i_socket_transport.h"
#include <vector>

namespace mpl::net
{

class socket_allocator
{
public:
    struct config_t
    {
        using port_range_t = std::pair<std::uint16_t, std::uint16_t>;
        port_range_t    port_range;

        config_t(const port_range_t& port_range = {});

        bool has_port_range() const;
    };

private:
    i_transport_factory&    m_transport_factory;
    config_t                m_config;

public:
    using socket_array_t = std::vector<i_socket_transport::u_ptr_t>;

    socket_allocator(i_transport_factory& transport_factory
                     , const config_t& config = {});

    i_socket_transport::u_ptr_t create_socket(const socket_address_t& socket_address = {});
    socket_array_t allocate_sockets(std::uint8_t count = 2
                                    , const socket_address_t& socket_address = {});



};

}

#endif // MPL_NET_SOCKET_ALLOCATOR_H
