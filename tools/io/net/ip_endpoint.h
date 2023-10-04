#ifndef IO_NET_IP_ENDPOINT_H
#define IO_NET_IP_ENDPOINT_H

#include "tools/io/io_base.h"
#include "ip_address.h"

namespace io
{

struct ip_endpoint_t
{
    using array_t = std::vector<ip_endpoint_t>;

    ip_address_t    address;
    port_t          port;

    struct hasher_t
    {
        std::size_t operator()(const ip_endpoint_t& endpoint) const;
    };

    static const ip_endpoint_t& undefined();
    static const ip_endpoint_t& any_v4();
    static const ip_endpoint_t& any_v6();

    static ip_endpoint_t from_string(const std::string& string_enpoint);

    ip_endpoint_t(const std::string& string_endpoint);

    ip_endpoint_t(const ip_address_t& address = {}
                  , port_t port = port_any);

    bool operator == (const ip_endpoint_t& other) const;
    bool operator != (const ip_endpoint_t& other) const;

    bool is_echo_port() const;
    bool is_discard_port() const;
    bool is_any_port() const;
    bool is_defined() const;
    bool is_v4() const;
    bool is_v6() const;

    bool is_valid() const;
    std::string to_string() const;

    std::size_t hash() const;

    ip_endpoint_t prev() const;
    ip_endpoint_t next() const;

    bool is_support(const ip_endpoint_t& other) const;
};

}

#endif // IO_NET_IP_ENDPOINT_H
