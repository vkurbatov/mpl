#include "utils/property_writer.h"
#include "utils/property_utils.h"

#include "net_types.h"

#include "socket/socket_endpoint.h"
#include "socket/udp_transport_params.h"

#include "ice/ice_auth_params.h"
#include "ice/ice_candidate.h"
#include "ice/ice_endpoint.h"
#include "ice/ice_transport_params.h"

#include "tls/tls_fingerprint.h"
#include "tls/tls_endpoint.h"
#include "tls/tls_transport_params.h"

#include "serial/serial_transport_params.h"

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

// udp_endpoint_t
template<>
bool utils::property::serialize(const udp_endpoint_t& value, i_property& property)
{
    property_writer write(property);
    return write.set("transport_id", value.transport_id)
            && write.set("address", value.socket_address.address)
            && write.set("port", value.socket_address.port, port_any);
}

template<>
bool utils::property::deserialize(udp_endpoint_t& value, const i_property &property)
{
    property_reader reader(property);
    if (reader.get("transport_id", transport_id_t::undefined) == value.transport_id)
    {
        return reader.get("address", value.socket_address.address)
                | reader.get("port", value.socket_address.port);
    }

    return false;
}

// tcp_endpoint_t
template<>
bool utils::property::serialize(const tcp_endpoint_t& value, i_property& property)
{
    property_writer write(property);
    return write.set("transport_id", value.transport_id)
            && write.set("address", value.socket_address.address)
            && write.set("port", value.socket_address.port, port_any);
}

template<>
bool utils::property::deserialize(tcp_endpoint_t& value, const i_property &property)
{
    property_reader reader(property);
    if (reader.get("transport_id", transport_id_t::undefined) == value.transport_id)
    {
        return reader.get("address", value.socket_address.address)
                | reader.get("port", value.socket_address.port);
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
    return write.set("transport_id", value.transport_id)
            && write.set("auth", value.auth)
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


// tls_fingerprint_t
template<>
bool utils::property::serialize(const tls_fingerprint_t& value, i_property& property)
{

    property_writer write(property);
    return write.set("method", value.method)
            && write.set("hash", value.hash);
}

template<>
bool utils::property::deserialize(tls_fingerprint_t& value, const i_property &property)
{
    property_reader reader(property);
    if (reader.get("method", value.method))
    {
        reader.get("hash", value.hash);
        return true;
    }

    return false;
}

// tls_endpoint_t
template<>
bool utils::property::serialize(const tls_endpoint_t& value, i_property& property)
{
    property_writer write(property);
    return write.set("transport_id", value.transport_id)
            && write.set("fingerprint", value.fingerprint);
}

template<>
bool utils::property::deserialize(tls_endpoint_t& value, const i_property &property)
{
    property_reader reader(property);
    if (reader.get("transport_id", transport_id_t::tls) == value.transport_id)
    {
        reader.get("fingerprint", value.fingerprint);
        return true;
    }

    return false;
}

// tls_transport_params_t
template<>
bool utils::property::serialize(const tls_transport_params_t& value, i_property& property)
{

    property_writer write(property);
    return write.set("role", value.role)
            && write.set("local_endpoint", value.local_endpoint)
            && write.set("remote_endpoint", value.remote_endpoint);
}

template<>
bool utils::property::deserialize(tls_transport_params_t& value, const i_property &property)
{
    property_reader reader(property);
    return reader.get("role", value.role)
            | reader.get("local_endpoint", value.local_endpoint)
            | reader.get("remote_endpoint", value.remote_endpoint);
}

// serial_endpoint_t
template<>
bool utils::property::serialize(const serial_endpoint_t& value, i_property& property)
{

    property_writer write(property);
    return write.set("transport_id", value.transport_id)
            && write.set("port_name", value.port_name);
}

template<>
bool utils::property::deserialize(serial_endpoint_t& value, const i_property &property)
{
    property_reader reader(property);
    if (reader.get("transport_id", transport_id_t::serial) == value.transport_id)
    {
        reader.get("port_name", value.port_name);
        return true;
    }
    return false;
}

// serial_transport_params_t
template<>
bool utils::property::serialize(const serial_transport_params_t& value, i_property& property)
{

    property_writer write(property);
    return write.set("baud_rate", value.serial_params.baud_rate)
            && write.set("char_size", value.serial_params.char_size)
            && write.set("stop_bits", value.serial_params.stop_bits)
            && write.set("parity", value.serial_params.parity)
            && write.set("flow_control", value.serial_params.flow_control)
            && write.set("endpoint", value.serial_endpoint);
}

template<>
bool utils::property::deserialize(serial_transport_params_t& value, const i_property &property)
{
    property_reader reader(property);
    return reader.get("baud_rate", value.serial_params.baud_rate)
            | reader.get("char_size", value.serial_params.char_size)
            | reader.get("stop_bits", value.serial_params.stop_bits)
            | reader.get("parity", value.serial_params.parity)
            | reader.get("flow_control", value.serial_params.flow_control)
            | reader.get("endpoint", value.serial_endpoint);
}

}
