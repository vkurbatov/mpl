cmake_minimum_required(VERSION 3.16)

set(TARGET mplnet)

set(SOURCES
    test.cpp
    endpoint.cpp
    enum_convert_impl.cpp
    enum_serialize_impl.cpp
    net_serialize_impl.cpp
    net_engine_impl.cpp
    net_utils.cpp

    socket/socket_endpoint.cpp
    socket/socket_packet_impl.cpp
    socket/udp_transport_factory.cpp
    socket/udp_transport_params.cpp

    stun/stun_mapped_headers.cpp
    stun/stun_attributes.cpp
    stun/stun_packet_impl.cpp
    stun/stun_message.cpp
    stun/stun_utils.cpp

    ice/ice_auth_params.cpp
    ice/ice_transaction.cpp
    ice/ice_controller.cpp
    ice/ice_candidate.cpp
    ice/ice_endpoint.cpp
    ice/ice_server_params.cpp
    ice/ice_transport_params.cpp
    ice/ice_gathering_command.cpp
    ice/ice_utils.cpp
    ice/ice_config.cpp
    ice/ice_transport_factory.cpp
)

set(PUBLIC_HEADERS
    net_types.h
    endpoint.h
    i_net_packet.h  
    i_transport_channel.h
    i_transport_factory.h
    i_net_engine.h
    socket/socket_types.h
    socket/socket_endpoint.h
    socket/i_socket_packet.h
    socket/i_socket_transport.h
    socket/i_udp_transport.h

    stun/stun_types.h
    stun/i_stun_packet.h
    stun/stun_attributes.h

    ice/ice_types.h
    ice/ice_auth_params.h
    ice/ice_candidate.h
    ice/ice_endpoint.h
    ice/ice_gathering_command.h
    ice/ice_server_params.h
    ice/i_ice_transport.h
    )

set(PRIVATE_HEADERS
    test.h
    net_engine_impl.h
    net_utils.h
    socket/socket_packet_impl.h
    socket/udp_transport_factory.h
    socket/udp_transport_params.h

    stun/stun_mapped_headers.h
    stun/stun_packet_impl.h
    stun/stun_message.h
    stun/stun_error_codes.h
    stun/stun_utils.h

    ice/ice_utils.h
    ice/ice_transaction.h
    ice/ice_controller.h
    ice/ice_transport_params.h
    ice/ice_config.h
    ice/ice_transport_factory.h
)


add_library(${TARGET}
    ${SOURCES}
    ${PRIVATE_HEADERS}
    ${PUBLIC_HEADERS}
)

