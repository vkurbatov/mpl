#ifndef IO_NET_IP_ENDPOINT_H
#define IO_NET_IP_ENDPOINT_H

#include "tools/io/io_base.h"
#include "tools/io/endpoint.h"
#include "ip_address.h"

namespace pt::io
{

struct ip_endpoint_t : public endpoint_t
{
    using array_t = std::vector<ip_endpoint_t>;

    ip_address_t    address;
    port_t          port;

    static const ip_endpoint_t& undefined();
    static const ip_endpoint_t& any_v4();
    static const ip_endpoint_t& any_v6();

    static ip_endpoint_t from_string(const std::string& string_enpoint);

    ip_endpoint_t(const std::string& string_endpoint);

    ip_endpoint_t(const ip_address_t& address = {}
                  , port_t port = port_any);

    ~ip_endpoint_t() = default;

    bool operator == (const ip_endpoint_t& other) const;

    bool operator == (const endpoint_t& other) const override;
    bool is_valid() const override;
    std::string to_string() const override;

    bool is_echo_port() const;
    bool is_discard_port() const;
    bool is_any_port() const;
    bool is_defined() const;
    bool is_v4() const;
    bool is_v6() const;


    std::size_t hash() const;

    ip_endpoint_t prev() const;
    ip_endpoint_t next() const;

    bool is_support(const ip_endpoint_t& other) const;
};

}

#endif // IO_NET_IP_ENDPOINT_H
