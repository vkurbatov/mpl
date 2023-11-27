#ifndef IO_SERIAL_ENDPOINT_H
#define IO_SERIAL_ENDPOINT_H

#include "tools/io/endpoint.h"
#include <string>

namespace pt::io
{

struct serial_endpoint_t : public endpoint_t
{
    std::string     port_name;

    serial_endpoint_t(const std::string_view& port_name = {});

    // endpoint_t interface
public:
    bool operator ==(const endpoint_t &other) const override;
    bool is_valid() const override;
    std::string to_string() const override;
};

}

#endif // IO_SERIAL_ENDPOINT_H