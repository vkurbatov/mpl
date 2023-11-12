#include "socket_allocator.h"
#include "utils/property_writer.h"
#include "utils/pointer_utils.h"

namespace mpl::net
{

socket_allocator::config_t::config_t(const port_range_t &port_range)
    : port_range(port_range)
{

}

bool socket_allocator::config_t::has_port_range() const
{
    return port_range.first < port_range.second;
}

socket_allocator::socket_allocator(i_transport_factory &transport_factory
                                   , const config_t &config)
    : m_transport_factory(transport_factory)
    , m_config(config)
{

}

i_socket_transport::u_ptr_t socket_allocator::create_socket(const socket_address_t& socket_address)
{
    if (auto socket_params = utils::property::create_property(property_type_t::object))
    {
        property_writer writer(*socket_params);
        writer.set("local_endpoint", udp_endpoint_t(socket_address));

        if (socket_address.is_any_port()
                && m_config.has_port_range())
        {
            for (socket_address_t address(socket_address.address, m_config.port_range.first)
                 ; address.port < m_config.port_range.second
                 ; address.port++)
            {
                writer.set("local_endpoint", udp_endpoint_t(address));
                if (auto socket = utils::static_pointer_cast<i_socket_transport>(m_transport_factory.create_transport(*socket_params)))
                {
                    if (socket->control(channel_control_t::open()))
                    {
                        return socket;
                    }
                }
            }
        }
        else
        {
            if (auto socket = utils::static_pointer_cast<i_socket_transport>(m_transport_factory.create_transport(*socket_params)))
            {
                if (socket->control(channel_control_t::open()))
                {
                    return socket;
                }
            }
        }
    }

    return nullptr;
}

socket_allocator::socket_array_t socket_allocator::allocate_sockets(uint8_t count
                                                                    , const socket_address_t &socket_address)
{
    if (count > 0)
    {
        std::size_t try_count = 50;
        auto address = socket_address;

        bool has_range = false;

        if (socket_address.is_any_port()
                && m_config.has_port_range())
        {
            has_range = true;
            address.port = m_config.port_range.first;
        }

        while(try_count -- > 0)
        {

            if (has_range
                    && (socket_address.port + count) > m_config.port_range.second)
            {
                break;
            }

            socket_allocator::socket_array_t sockets(count);
            if (auto socket = create_socket(address))
            {
                auto addr = socket->local_endpoint().socket_address;
                std::uint16_t idx = addr.port % static_cast<std::uint8_t>(count);
                sockets[idx] = std::move(socket);
                addr.port -= idx;

                bool ok = true;
                for (std::uint16_t i = 0; i < count; i++, addr.port++)
                {
                    if (i != idx)
                    {
                        sockets[i] = create_socket(addr);
                        if (sockets[i] == nullptr)
                        {
                            ok = false;
                            break;
                        }
                    }
                }

                if (ok)
                {
                    return sockets;
                }

                if (!address.is_any_port())
                {
                    address.port += count;
                }
            }
        }
    }

    return { };
}


}
