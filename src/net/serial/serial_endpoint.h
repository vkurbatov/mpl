#ifndef MPL_NET_SERIAL_ENDPOINT_H
#define MPL_NET_SERIAL_ENDPOINT_H

#include "net/endpoint.h"
#include <string>

namespace mpl::net
{

struct serial_endpoint_t : public endpoint_t
{
    std::string     port_name;

    serial_endpoint_t(const std::string_view& port_name = {});

    // endpoint_t interface
public:
    bool operator ==(const endpoint_t &other) const override;
    bool is_valid() const override;
};

}

#endif // MPL_NET_SERIAL_ENDPOINT_H
