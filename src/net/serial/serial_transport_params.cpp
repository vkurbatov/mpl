#include "serial_transport_params.h"

namespace mpl::net
{

serial_transport_params_t::serial_transport_params_t(const pt::io::serial_link_config_t &serial_params
                                                     , const serial_endpoint_t &serial_endpoint)
    : serial_params(serial_params)
    , serial_endpoint(serial_endpoint)
{

}

bool serial_transport_params_t::operator ==(const serial_transport_params_t &other) const
{
    return serial_params == other.serial_params
            && serial_endpoint == other.serial_endpoint;
}

bool serial_transport_params_t::operator !=(const serial_transport_params_t &other) const
{
    return ! operator == (other);
}

bool serial_transport_params_t::is_valid() const
{
    return serial_params.is_valid()
            && serial_endpoint.is_valid();
}



}
