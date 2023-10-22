#include "ip_endpoint.h"
#include "tools/utils/string_base.h"
#include "tools/utils/convert_base.h"

template<>
struct std::hash<pt::io::ip_endpoint_t>
{
    std::size_t operator()(const pt::io::ip_endpoint_t& s) const noexcept
    {
        return s.hash();
    }
};

namespace pt::io
{

const ip_endpoint_t &ip_endpoint_t::undefined()
{
    static const ip_endpoint_t single_endpoint_undefined;
    return single_endpoint_undefined;
}

const ip_endpoint_t &ip_endpoint_t::any_v4()
{
    static const ip_endpoint_t single_endpoint_v4(ip_address_t::ip4_address_t::any()
                                                  , port_any);
    return single_endpoint_v4;
}

const ip_endpoint_t &ip_endpoint_t::any_v6()
{
    static const ip_endpoint_t single_endpoint_v6(ip_address_t::ip6_address_t::any()
                                                  , port_any);
    return single_endpoint_v6;
}

ip_endpoint_t ip_endpoint_t::from_string(const std::string &string_enpoint)
{
    if (!string_enpoint.empty())
    {
        ip_address_t ip_address;
        if (ip_address.from_string(string_enpoint))
        {
            return ip_endpoint_t(ip_address);
        }

        // ipv6
        std::vector<std::string> args;
        if (string_enpoint.front() == '[')
        {
            args = pt::utils::split_lines(string_enpoint, "]");
            if (args.size() > 0)
            {
                args[0] = args[0].substr(1);
            }
            if (args.size() > 1)
            {
                args[1] = args[1].substr(1);
            }
        }
        else
        {
            args = pt::utils::split_lines(string_enpoint, ":");
        }

        if (args.size() == 1
                || args.size() == 2)
        {

            ip_endpoint_t result_endpoint(ip_address_t(args[0])
                                          , port_any);

            if (result_endpoint.address.is_valid())
            {
                if (args.size() == 2)
                {
                    if (pt::utils::convert(args[1]
                                      , result_endpoint.port))
                    {
                        return result_endpoint;
                    }
                }
                else
                {
                    return result_endpoint;
                }
            }
        }
    }

    return {};
}

ip_endpoint_t::ip_endpoint_t(const std::string &string_endpoint)
    : ip_endpoint_t(from_string(string_endpoint))
{

}

ip_endpoint_t::ip_endpoint_t(const ip_address_t &address
                             , port_t port)
    : endpoint_t(type_t::ip)
    , address(address)
    , port(port)
{

}

bool ip_endpoint_t::operator ==(const ip_endpoint_t &other) const
{
    return address == other.address
            && port == other.port;
}

bool ip_endpoint_t::operator ==(const endpoint_t &other) const
{
    return other.type == type_t::ip
            && *this == static_cast<const ip_endpoint_t&>(other);
}


bool ip_endpoint_t::is_echo_port() const
{
    return port == port_echo;
}

bool ip_endpoint_t::is_discard_port() const
{
    return port == port_discard;
}

bool ip_endpoint_t::is_any_port() const
{
    return port == port_any;
}

bool ip_endpoint_t::is_defined() const
{
    return !is_any_port();
}

bool ip_endpoint_t::is_v4() const
{
    return address.is_v4();
}

bool ip_endpoint_t::is_v6() const
{
    return address.is_v6();
}

std::string ip_endpoint_t::to_string() const
{
    switch(address.version)
    {
        case ip_version_t::ip4:
            return address.to_string()
                    .append(":")
                    .append(std::to_string(port));
        break;
        case ip_version_t::ip6:
            return std::string("[")
                    .append(address.to_string())
                    .append("]:").append(std::to_string(port));
        break;
        default: ;
    }

    return std::to_string(port);
}

std::size_t ip_endpoint_t::hash() const
{
    return address.hash()
            ^ std::hash<std::uint16_t>()(port);
}

ip_endpoint_t ip_endpoint_t::prev() const
{
    return ip_endpoint_t(address
                         , port - 1);
}

ip_endpoint_t ip_endpoint_t::next() const
{
    return ip_endpoint_t(address
                         , port + 1);
}

bool ip_endpoint_t::is_support(const ip_endpoint_t &other) const
{
    return (address.version == other.address.version)
            && (address.is_any() || address == other.address)
            && (is_any_port() || port == other.port);
}

bool ip_endpoint_t::is_valid() const
{
    return address.is_valid();
}

}


