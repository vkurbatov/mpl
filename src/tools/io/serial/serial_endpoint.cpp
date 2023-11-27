#include "serial_endpoint.h"

namespace pt::io
{

serial_endpoint_t::serial_endpoint_t(const std::string_view &port_name)
    : endpoint_t(type_t::serial)
    , port_name(port_name)
{

}

bool serial_endpoint_t::operator ==(const endpoint_t &other) const
{
    return other.type == type_t::serial
            && port_name == static_cast<const serial_endpoint_t&>(other).port_name;
}

bool serial_endpoint_t::is_valid() const
{
    return type == type_t::serial
            && !port_name.empty();
}

std::string serial_endpoint_t::to_string() const
{
    return std::string("serial:").append(port_name);
}

}