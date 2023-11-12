#include "net_utils.h"
#include "ip_endpoint.h"
#include <boost/asio/ip/host_name.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/tcp.hpp>

#ifdef LINUX

#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

#endif


namespace pt::io::utils
{

namespace detail
{

inline struct hostent* get_host_by_name(const std::string_view& name
                                        , ip_version_t version)
{
    if (!name.empty())
    {
        switch(version)
        {
            case ip_version_t::ip4:
                return gethostbyname2(name.data()
                                      , AF_INET);
            break;
            case ip_version_t::ip6:
                return gethostbyname2(name.data()
                                      , AF_INET6);
            break;
            default:
                return gethostbyname(name.data());
        }
    }

    return nullptr;
}

}

template<>
boost::asio::ip::address convert(const ip_address_t& address)
{
    switch(address.version)
    {
        case ip_version_t::ip4:
            return boost::asio::ip::address_v4(address.ip4_address.address);
        break;
        case ip_version_t::ip6:
            return boost::asio::ip::address_v6(address.ip6_address.address);
        break;
        default : ;
    }

    return {};
}

template<>
ip_address_t convert(const boost::asio::ip::address& address)
{
    if (address.is_v4())
    {
        return ip_address_t::ip4_address_t(address.to_v4().to_uint());
    }
    else if (address.is_v6())
    {
        return ip_address_t::ip6_address_t(address.to_v6().to_bytes());
    }

    return {};
}

template<>
boost::asio::ip::udp::endpoint convert(const ip_endpoint_t& ip_endpoint)
{
    return { convert<boost::asio::ip::address>(ip_endpoint.address), ip_endpoint.port };
}

template<>
ip_endpoint_t convert(const boost::asio::ip::udp::endpoint& endpoint)
{
    return { convert<ip_address_t>(endpoint.address()), endpoint.port() };
}


template<>
boost::asio::ip::tcp::endpoint convert(const ip_endpoint_t& ip_endpoint)
{
    return { convert<boost::asio::ip::address>(ip_endpoint.address), ip_endpoint.port };
}

template<>
ip_endpoint_t convert(const boost::asio::ip::tcp::endpoint& endpoint)
{
    return { convert<ip_address_t>(endpoint.address()), endpoint.port() };
}

std::string get_host_name()
{
    return boost::asio::ip::host_name();
}

std::vector<ip_address_t> get_local_address_list(ip_version_t ip_version)
{
    std::vector<ip_address_t>   address_list;

    for (const auto& n : get_net_info(ip_version))
    {
        address_list.emplace_back(n.ip_address);
    }

    return address_list;
}

std::vector<net_info_t> get_net_info(ip_version_t ip_version)
{
    std::vector<net_info_t>   net_list;
#ifdef LINUX
    struct ifaddrs * if_addr_record = nullptr;

    getifaddrs(&if_addr_record );

    auto idx = 0;

    for (auto ifa = if_addr_record; ifa != nullptr; ifa = ifa->ifa_next, idx++)
    {
        if (!ifa->ifa_addr)
        {
            continue;
        }

        std::string net_name;
        if (ifa->ifa_name != nullptr)
        {
            net_name = ifa->ifa_name;
        }

        switch(ifa->ifa_addr->sa_family)
        {
            case AF_INET:
            {
                if (ip_version == ip_version_t::undefined
                        || ip_version == ip_version_t::ip4)
                {
                    net_info_t net_info(idx
                                        , std::move(net_name)
                                        , ip_address_t::ip4_address_t(ntohl(((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr)));
                    net_list.emplace_back(std::move(net_info));
                }
            }
            break;
            case AF_INET6:
            {
                if (ip_version == ip_version_t::undefined
                        || ip_version == ip_version_t::ip6)
                {
                    net_info_t net_info(idx
                                        , std::move(net_name)
                                        , ip_address_t::ip6_address_t(ip_address_t::ip6_address_t(&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr)));
                    net_list.emplace_back(std::move(net_info));
                }
            }
            break;
        }
    }

    if (if_addr_record != nullptr)
    {
        freeifaddrs(if_addr_record);
    }

#endif
    return net_list;
}

ip_address_t get_host_by_name(const std::string_view &name
                              , ip_version_t ip_version)
{
    if (auto host_entry = detail::get_host_by_name(name
                                                   , ip_version))
    {
        if (const void* address = host_entry->h_addr_list[0])
        {
            switch(host_entry->h_addrtype)
            {
                case AF_INET:
                {
                    return ip_address_t::ip4_address_t(ntohl(*static_cast<const std::uint32_t*>(address)));
                }
                break;
                case AF_INET6:
                {
                    return ip_address_t::ip6_address_t(address);
                }
                break;
                default:;
            }
        }
    }

    return {};
}


}
