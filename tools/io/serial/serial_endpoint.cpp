#include "serial_endpoint.h"

namespace io
{

serial_endpoint_t::serial_endpoint_t(const std::string_view &port_name)
    : port_name(port_name)
{

}

bool serial_endpoint_t::operator ==(const endpoint_t &other) const
{
    return other.type == endpoint_type_t::serial
            && port_name == static_cast<const serial_endpoint_t&>(other).port_name;
}

bool serial_endpoint_t::is_valid() const
{
    return type == endpoint_type_t::serial
            && !port_name.empty();
}

}
