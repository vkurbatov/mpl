#ifndef IO_SERIAL_ENDPOINT_H
#define IO_SERIAL_ENDPOINT_H

#include "tools/io/io_base.h"
#include <string>

namespace io
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

#endif // IO_SERIAL_ENDPOINT_H
