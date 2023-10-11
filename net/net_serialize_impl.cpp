#include "utils/property_writer.h"
#include "utils/property_utils.h"

#include "net_types.h"

#include "socket_endpoint.h"
#include "udp_transport_params.h"

namespace mpl
{

using namespace net;

// ip_address_t
template<>
bool utils::property::serialize(const ip_address_t& value, i_property& property)
{
    if (value.is_valid())
    {
        property_writer writer(property);
        return writer.set("ip", value.to_string())
                && writer.set("version", value.version);
    }
    return false;
}

/*
template<>
i_property::u_ptr_t utils::property::serialize(const ip_address_t& value)
{
    if (value.is_valid())
    {
        if (auto object = utils::property::create_property(property_type_t::object))
        {
            if (serialize(value
                          , *object))
            {
                return object;
            }
        }
    }
    return nullptr;
}*/

template<>
bool utils::property::deserialize(ip_address_t& value
                                  , const i_property &property)
{
    std::string address_string;

    property_reader reader(property);
    ip_version_t version = reader.get("version", ip_version_t::undefined);

    if (reader.get("ip", address_string))
    {
        return value.from_string(address_string
                                 , version);
    }

    return false;
}

// socket_endpoint_t
template<>
bool utils::property::serialize(const socket_endpoint_t& value, i_property& property)
{
    property_writer write(property);
    return write.set("transport_id", value.transport_id)
            && write.set("address", value.ip_endpoint.address)
            && write.set("port", value.ip_endpoint.port, port_any);
}

template<>
bool utils::property::deserialize(socket_endpoint_t& value, const i_property &property)
{
    property_reader reader(property);
    if (reader.get("transport_id", transport_id_t::undefined) == value.transport_id)
    {
        return reader.get("address", value.ip_endpoint.address)
                | reader.get("address", value.ip_endpoint.port);
    }

    return false;
}

// socket_options_t
template<>
bool utils::property::serialize(const socket_options_t& value, i_property& property)
{
    property_writer write(property);
    return write.set("reuse_address", value.reuse_address)
            && write.set("reuse_port", value.reuse_port);
}

template<>
bool utils::property::deserialize(socket_options_t& value, const i_property &property)
{
    property_reader reader(property);
    return reader.get("reuse_address", value.reuse_address)
            | reader.get("reuse_port", value.reuse_port);

}

// udp_transport_params_t
template<>
bool utils::property::serialize(const udp_transport_params_t& value, i_property& property)
{
    property_writer write(property);
    return write.set("local_endpoint", value.local_endpoint, {})
            && write.set("remote_endpoint", value.remote_endpoint, {})
            && write.set("options", value.options, {});
}

template<>
bool utils::property::deserialize(udp_transport_params_t& value, const i_property &property)
{
    property_reader reader(property);
    return reader.get("local_endpoint", value.local_endpoint)
            | reader.get("remote_endpoint", value.remote_endpoint)
            | reader.get("options", value.options);

}



}
