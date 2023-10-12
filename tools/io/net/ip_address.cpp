#include "ip_address.h"
#include <boost/asio/ip/address_v4.hpp>
#include <boost/asio/ip/address_v6.hpp>
#include <boost/asio/ip/address.hpp>

#include "net_utils.h"

namespace io
{

namespace detail
{

boost::asio::ip::address address_from_string(const std::string& string_address
                                             , boost::system::error_code& error_code
                                             , ip_version_t version = ip_version_t::undefined)
{
    {
        switch(version)
        {
            case ip_version_t::ip4:
                return boost::asio::ip::make_address_v4(string_address, error_code);
            break;
            case ip_version_t::ip6:
                return boost::asio::ip::make_address_v6(string_address, error_code);
            break;
            default:;
        }
    }

    return boost::asio::ip::make_address(string_address, error_code);
}

}

const ip_address_t::ip4_address_t &ip_address_t::ip4_address_t::any()
{
    static const ip4_address_t single_any_address(boost::asio::ip::address_v4::any().to_uint());
    return single_any_address;
}

const ip_address_t::ip4_address_t &ip_address_t::ip4_address_t::loopback()
{
    static const ip4_address_t single_loopback_address(boost::asio::ip::address_v4::loopback().to_uint());
    return single_loopback_address;
}

const ip_address_t::ip4_address_t &ip_address_t::ip4_address_t::broadcast()
{
    static const ip4_address_t single_broadcast_address(boost::asio::ip::address_v4::broadcast().to_uint());
    return single_broadcast_address;
}

bool ip_address_t::ip4_address_t::from_string(const std::string &string_address
                                              , ip_address_t::ip4_address_t &ip4_address)
{
    boost::system::error_code error_code;
    auto ip_addr = boost::asio::ip::address_v4::from_string(string_address, error_code);
    if (!error_code.failed())
    {
        ip4_address.address = ip_addr.to_uint();
        return true;
    }

    return false;
}

ip_address_t::ip4_address_t::ip4_address_t(uint32_t address)
    : address(address)
{

}

ip_address_t::ip4_address_t::ip4_address_t(const ip_address_t::ip4_address_t::ip4_address_array_t &address)
    : address(boost::asio::ip::address_v4(address).to_uint())
{

}

ip_address_t::ip4_address_t::ip4_address_t(const std::string &string_address)
    : address(boost::asio::ip::address_v4::any().to_uint())
{
    from_string(string_address, *this);
}

bool ip_address_t::ip4_address_t::operator ==(const ip_address_t::ip4_address_t &other) const
{
    return address == other.address;
}

bool ip_address_t::ip4_address_t::operator !=(const ip_address_t::ip4_address_t &other) const
{
    return !operator == (other);
}

std::string ip_address_t::ip4_address_t::to_string() const
{
    return boost::asio::ip::address_v4(address).to_string();
}

ip_address_t::ip4_address_t::ip4_address_array_t ip_address_t::ip4_address_t::to_array() const
{
    return boost::asio::ip::address_v4(address).to_bytes();
}

std::size_t ip_address_t::ip4_address_t::hash() const
{
    return std::hash<std::uint32_t>()(address);
}

const ip_address_t::ip6_address_t &ip_address_t::ip6_address_t::any()
{
    static const ip6_address_t single_any_address(boost::asio::ip::address_v6::any().to_bytes());
    return single_any_address;
}

const ip_address_t::ip6_address_t &ip_address_t::ip6_address_t::loopback()
{
    static const ip6_address_t single_loopback_address(boost::asio::ip::address_v6::any().to_bytes());
    return single_loopback_address;
}

bool ip_address_t::ip6_address_t::from_string(const std::string &string_address
                                              , ip_address_t::ip6_address_t &ip6_address)

{
    boost::system::error_code error_code;
    auto ip_addr = boost::asio::ip::address_v6::from_string(string_address, error_code);
    if (!error_code.failed())
    {
        ip6_address.address = ip_addr.to_bytes();
        return true;
    }

    return false;
}

ip_address_t::ip6_address_t::ip6_address_t(const ip_address_t::ip6_address_t::ip6_address_array_t &address)
    : address(address)
{

}

ip_address_t::ip6_address_t::ip6_address_t(const void *array_pointer)
{
    std::memcpy(address.data(), array_pointer, address.size());
}

ip_address_t::ip6_address_t::ip6_address_t(const std::string &string_address)
    : address(boost::asio::ip::address_v6(address).to_bytes())
{
    from_string(string_address, *this);
}

bool ip_address_t::ip6_address_t::operator ==(const ip_address_t::ip6_address_t &other) const
{
    return address == other.address;
}

bool ip_address_t::ip6_address_t::operator !=(const ip_address_t::ip6_address_t &other) const
{
    return !operator == (other);
}

std::string ip_address_t::ip6_address_t::to_string() const
{
    return boost::asio::ip::address_v6(address).to_string();
}

std::size_t ip_address_t::ip6_address_t::hash() const
{
    return std::hash<std::uint64_t>()(*reinterpret_cast<const uint64_t*>(&address[0]))
            ^ std::hash<std::uint64_t>()(*reinterpret_cast<const uint64_t*>(&address[8]));
}

const ip_address_t &ip_address_t::undefined()
{
    static const ip_address_t single_undefined_address;
    return single_undefined_address;
}

const ip_address_t &ip_address_t::any_v4()
{
    static const ip_address_t single_any_v4_address(ip4_address_t::any());
    return single_any_v4_address;
}

const ip_address_t &ip_address_t::loopback_v4()
{
    static const ip_address_t single_loopback_v4_address(ip4_address_t::loopback());
    return single_loopback_v4_address;
}

const ip_address_t &ip_address_t::broadcast_v4()
{
    static const ip_address_t single_broadcast_v4_address(ip4_address_t::broadcast());
    return single_broadcast_v4_address;
}

const ip_address_t &ip_address_t::any_v6()
{
    static const ip_address_t single_broadcast_v6_address(ip6_address_t::any());
    return single_broadcast_v6_address;
}

const ip_address_t &ip_address_t::loopback_v6()
{
    static const ip_address_t single_broadcast_v6_address(ip6_address_t::loopback());
    return single_broadcast_v6_address;
}

bool ip_address_t::from_string(const std::string &string_address
                               , ip_address_t &address
                               , ip_version_t version)
{

    boost::system::error_code error_code;
    auto ip_address = detail::address_from_string(string_address
                                                  , error_code
                                                  , version);
    if (!error_code.failed())
    {
        if (ip_address.is_v4())
        {
            address = ip4_address_t(ip_address.to_v4().to_uint());
        }
        else
        {
            address = ip6_address_t(ip_address.to_v6().to_bytes());
        }

        return true;
    }
    else
    {
        auto resolved_address = utils::get_host_by_name(string_address);
        if (resolved_address.is_valid())
        {
            address = std::move(resolved_address);
            return true;
        }
    }

    return false;
}

ip_address_t ip_address_t::build_from_string(const std::string &string_address
                                             , ip_version_t version)
{
    ip_address_t ip_address;
    from_string(string_address
                , ip_address
                , version);
    return ip_address;
}

ip_address_t::ip_address_t()
    : version(ip_version_t::undefined)
{

}

ip_address_t::ip_address_t(const std::string &string_address
                           , ip_version_t version)
    : ip_address_t()
{
    from_string(string_address
                , *this
                , version);
}

ip_address_t::ip_address_t(const ip_address_t::ip4_address_t &ip4_address)
    : version(ip_version_t::ip4)
    , ip4_address(ip4_address)
{

}

ip_address_t::ip_address_t(const ip_address_t::ip6_address_t &ip6_address)
    : version(ip_version_t::ip6)
    , ip6_address(ip6_address)
{

}

bool ip_address_t::operator ==(const ip_address_t &other) const
{
    if (version == other.version)
    {
        switch(version)
        {
            case ip_version_t::ip4:
                return ip4_address == other.ip4_address;
            break;
            case ip_version_t::ip6:
                return ip6_address == other.ip6_address;
            break;
            default:
                return true;
        }
    }

    return false;
}

bool ip_address_t::operator !=(const ip_address_t &other) const
{
    return ! operator == (other);
}

ip_address_t::~ip_address_t()
{
    switch(version)
    {
        case ip_version_t::ip4:
            ip4_address.~ip4_address_t();
        break;
        case ip_version_t::ip6:
            ip6_address.~ip6_address_t();
        break;
        default: ;
    }
}

bool ip_address_t::is_any() const
{
    switch(version)
    {
        case ip_version_t::ip4:
            return ip4_address == ip4_address_t::any();
        break;
        case ip_version_t::ip6:
            return ip6_address == ip6_address_t::any();
        break;
        default: ;
    }

    return false;
}

bool ip_address_t::is_loopback() const
{
    switch(version)
    {
        case ip_version_t::ip4:
            return ip4_address == ip4_address_t::loopback();
        break;
        case ip_version_t::ip6:
            return ip6_address == ip6_address_t::loopback();
        break;
        default: ;
    }

    return false;
}

bool ip_address_t::is_broadcast() const
{
    return version == ip_version_t::ip4
            && ip4_address == ip4_address_t::broadcast();
}

bool ip_address_t::is_v4() const
{
    return version == ip_version_t::ip4;
}

bool ip_address_t::is_v6() const
{
    return version == ip_version_t::ip6;
}


bool ip_address_t::is_valid() const
{
    return version != ip_version_t::undefined;
}

bool ip_address_t::is_defined() const
{
    return (*this) != undefined();
}

std::string ip_address_t::to_string() const
{
    switch(version)
    {
        case ip_version_t::ip4:
            return ip4_address.to_string();
        break;
        case ip_version_t::ip6:
            return ip6_address.to_string();
        break;
        default: ;
    }

    return {};
}

bool ip_address_t::from_string(const std::string &string_address
                               , ip_version_t version)
{
    return from_string(string_address
                       , *this
                       , version);
}

std::size_t ip_address_t::hash() const
{
    switch(version)
    {
        case ip_version_t::ip4:
            return ip4_address.hash();
        break;
        case ip_version_t::ip6:
            return ip6_address.hash();
        break;
        default: ;
    }

    return 0;
}

}
