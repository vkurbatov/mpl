#include "utils/property_writer.h"
#include "utils/property_utils.h"

#include "net_types.h"

#include "socket/socket_endpoint.h"
#include "socket/udp_transport_params.h"

#include "ice/ice_auth_params.h"
#include "ice/ice_candidate.h"
#include "ice/ice_endpoint.h"
#include "ice/ice_transport_params.h"

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

// socket_address_t
template<>
bool utils::property::serialize(const socket_address_t& value, i_property& property)
{

    property_writer writer(property);
    return writer.set("address", value.address)
            && writer.set("port", value.port);
}

template<>
bool utils::property::deserialize(socket_address_t& value, const i_property &property)
{
    property_reader deserializer(property);
    if (deserializer.get("address", value.address))
    {
        return deserializer.get("port", value.port);
    }

    return false;
}

// socket_endpoint_t
template<>
bool utils::property::serialize(const socket_endpoint_t& value, i_property& property)
{
    property_writer write(property);
    return write.set("transport_id", value.transport_id)
            && write.set("address", value.socket_address.address)
            && write.set("port", value.socket_address.port, port_any);
}

template<>
bool utils::property::deserialize(socket_endpoint_t& value, const i_property &property)
{
    property_reader reader(property);
    if (reader.get("transport_id", transport_id_t::undefined) == value.transport_id)
    {
        return reader.get("address", value.socket_address.address)
                | reader.get("address", value.socket_address.port);
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
    return write.set("local_endpoint", value.local_endpoint)
            && write.set("remote_endpoint", value.remote_endpoint)
            && write.set("options", value.options);
}

template<>
bool utils::property::deserialize(udp_transport_params_t& value, const i_property &property)
{
    property_reader reader(property);
    return reader.get("local_endpoint", value.local_endpoint)
            | reader.get("remote_endpoint", value.remote_endpoint)
            | reader.get("options", value.options);

}

// ice_auth_params_t
template<>
bool utils::property::serialize(const ice_auth_params_t& value, i_property& property)
{
    property_writer write(property);
    return write.set("ufrag", value.ufrag)
            && write.set("password", value.password);
}

template<>
bool utils::property::deserialize(ice_auth_params_t& value, const i_property &property)
{
    property_reader reader(property);
    return reader.get("ufrag", value.ufrag)
            | reader.get("password", value.password);

}

// ice_candidate_t
template<>
bool utils::property::serialize(const ice_candidate_t& value
                                    , i_property& property)
{

    property_writer write(property);
    return write.set("foundation", value.foundation)
            && write.set("component_id", value.component_id)
            && write.set("transport", value.transport)
            && write.set("priority", value.priority)
            && write.set("connection_address", value.connection_address)
            && write.set("type", value.type)
            && write.set("relayed_address", value.relayed_address)
            && write.set("tcp_type", value.tcp_role)
            && write.set("generation", value.generation)
            && write.set("ufrag", value.ufrag)
            && write.set("network_cost", value.network_cost);
}

template<>
bool utils::property::deserialize(ice_candidate_t& value
                                        , const i_property &property)
{
    property_reader reader(property);
    if (reader.get("foundation", value.foundation)
        && reader.get("component_id", value.component_id)
        && reader.get("transport", value.transport)
        && reader.get("priority", value.priority)
        && reader.get("connection_address", value.connection_address))
    {
        reader.get("type", value.type);
        reader.get("relayed_address", value.relayed_address);
        reader.get("tcp_type", value.tcp_role);
        reader.get("generation", value.generation);
        reader.get("ufrag", value.ufrag);
        reader.get("network_cost", value.network_cost);
        return true;
    }

    return false;
}

// ice_endpoint_t
template<>
bool utils::property::serialize(const ice_endpoint_t& value
                                    , i_property& property)
{

    property_writer write(property);
    return write.set("auth", value.auth)
            && write.set("candidates", value.candidates);
}

template<>
bool utils::property::deserialize(ice_endpoint_t& value
                                  , const i_property &property)
{
    property_reader reader(property);
    if (reader.get("auth", value.auth)
        | reader.get("candidates", value.candidates))
    {
        return true;
    }

    return false;
}

// ice_transport_params_t
template<>
bool utils::property::serialize(const ice_transport_params_t& value
                                    , i_property& property)
{
    property_writer write(property);
    return write.set("sockets", value.sockets)
            && write.set("component_id", value.component_id)
            && write.set("mode", value.mode)
            && write.set("local_endpoint", value.local_endpoint)
            && write.set("remote_endpoint", value.remote_endpoint);
}

template<>
bool utils::property::deserialize(ice_transport_params_t& value
                                  , const i_property &property)
{
    property_reader reader(property);
    return reader.get("sockets", value.sockets)
            | reader.get("component_id", value.component_id)
            | reader.get("mode", value.mode)
            | reader.get("local_endpoint", value.local_endpoint)
            | reader.get("remote_endpoint", value.remote_endpoint);
}



}
