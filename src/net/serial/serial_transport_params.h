#ifndef MPL_NET_SERIAL_TRANSPORT_PARAMS_H
#define MPL_NET_SERIAL_TRANSPORT_PARAMS_H

#include "serial_types.h"
#include "tools/io/serial/serial_link_config.h"
#include "serial_endpoint.h"

namespace mpl::net
{

struct serial_transport_params_t
{
    pt::io::serial_link_config_t    serial_params;
    serial_endpoint_t               serial_endpoint;

    serial_transport_params_t(const pt::io::serial_link_config_t& serial_params = {}
                              , const serial_endpoint_t& serial_endpoint = {});

    bool operator == (const serial_transport_params_t& other) const;
    bool operator != (const serial_transport_params_t& other) const;

    bool is_valid() const;
};

}

#endif // MPL_NET_SERIAL_TRANSPORT_PARAMS_H
