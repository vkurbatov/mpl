#include "serial_endpoint.h"

namespace mpl::net
{

serial_endpoint_t::serial_endpoint_t(const std::string_view &port_name)
    : endpoint_t(transport_id_t::serial)
    , port_name(port_name)
{

}

bool serial_endpoint_t::operator ==(const endpoint_t &other) const
{
    return other.transport_id == transport_id_t::serial
            && port_name == static_cast<const serial_endpoint_t&>(other).port_name;
}

bool serial_endpoint_t::is_valid() const
{
    return transport_id == transport_id_t::serial
            && !port_name.empty();
}



}
