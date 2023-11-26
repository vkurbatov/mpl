#ifndef IO_NET_IP_ADDRESS_H
#define IO_NET_IP_ADDRESS_H

#include <cstdint>
#include <array>
#include <string>
#include "net_types.h"

namespace pt::io
{

struct ip_address_t
{
    struct ip4_address_t
    {
        using ip4_address_array_t = std::array<std::uint8_t, 4>;
        std::uint32_t               address;

        static const ip4_address_t& any();
        static const ip4_address_t& loopback();
        static const ip4_address_t& broadcast();

        static bool from_string(const std::string& string_address
                                , ip4_address_t& ip4_address);

        ip4_address_t(std::uint32_t address = any().address);
        ip4_address_t(const ip4_address_array_t& address);
        ip4_address_t(const std::string& string_address);
        //~ip4_address_t() = default;

        bool operator == (const ip4_address_t& other) const;
        bool operator != (const ip4_address_t& other) const;
        std::string to_string() const;
        ip4_address_array_t to_array() const;

        std::size_t hash() const;
    };

    struct ip6_address_t
    {
        using ip6_address_array_t = std::array<std::uint8_t, 16>;
        ip6_address_array_t         address;

        static const ip6_address_t& any();
        static const ip6_address_t& loopback();

        static bool from_string(const std::string& string_address
                                , ip6_address_t& ip6_address);

        ip6_address_t(const ip6_address_array_t& address = {});
        ip6_address_t(const void* array_pointer);
        ip6_address_t(const std::string& string_address);
        //~ip6_address_t() = default;
        bool operator == (const ip6_address_t& other) const;
        bool operator != (const ip6_address_t& other) const;
        std::string to_string() const;

        std::size_t hash() const;

    };

    ip_version_t                        version;

    union
    {
        ip4_address_t                   ip4_address;
        ip6_address_t                   ip6_address;
    };

    static const ip_address_t& undefined();
    static const ip_address_t& any_v4();
    static const ip_address_t& loopback_v4();
    static const ip_address_t& broadcast_v4();

    static const ip_address_t& any_v6();
    static const ip_address_t& loopback_v6();

    static bool from_string(const std::string& string_address
                            , ip_address_t& address
                            , ip_version_t version = ip_version_t::undefined);
    static ip_address_t build_from_string(const std::string& string_address
                                           , ip_version_t version = ip_version_t::undefined);

    ip_address_t();
    ip_address_t(const std::string& string_address
                 , ip_version_t version = ip_version_t::undefined);
    ip_address_t(const ip4_address_t& ip4_address);
    ip_address_t(const ip6_address_t& ip6_address);

    bool operator == (const ip_address_t& other) const;
    bool operator != (const ip_address_t& other) const;

    ~ip_address_t();

    bool is_any() const;
    bool is_loopback() const;
    bool is_broadcast() const;
    bool is_v4() const;
    bool is_v6() const;

    bool is_valid() const;
    bool is_defined() const;
    std::string to_string() const;
    bool from_string(const std::string& string_address
                     , ip_version_t version = ip_version_t::undefined);

    std::size_t hash() const;
};


}

#endif // IO_NET_IP_ADDRESS_H
