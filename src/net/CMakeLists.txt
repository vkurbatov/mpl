cmake_minimum_required(VERSION 3.16)

set(TARGET mplnet)

# socket sources
set(SOCKET_SOURCES
    socket/socket_endpoint.cpp
    socket/socket_packet_impl.cpp
    socket/udp_transport_factory.cpp
    socket/udp_transport_params.cpp
    socket/socket_allocator.cpp
    socket/socket_packet_builder_impl.cpp
    )

set(SOCKET_PUBLIC_HEADERS
    socket/socket_types.h
    socket/socket_endpoint.h
    socket/i_socket_packet.h
    socket/i_socket_packet_builder.h
    socket/i_socket_transport.h
    socket/i_udp_transport.h
    )

set(SOCKET_PRIVATE_HEADERS
    socket/socket_packet_impl.h
    socket/udp_transport_factory.h
    socket/udp_transport_params.h
    socket/socket_allocator.h
    socket/socket_packet_builder_impl.h
    )

# stun sources
set(STUN_SOURCES
    stun/stun_mapped_headers.cpp
    stun/stun_attributes.cpp
    stun/stun_packet_impl.cpp
    stun/stun_message.cpp
    stun/stun_utils.cpp
    )

set(STUN_PUBLIC_HEADERS
    stun/stun_types.h
    stun/i_stun_packet.h
    stun/stun_attributes.h
    )

set(STUN_PRIVATE_HEADERS
    stun/stun_mapped_headers.h
    stun/stun_packet_impl.h
    stun/stun_message.h
    stun/stun_error_codes.h
    stun/stun_utils.h
    )

# ice sources
set(ICE_SOURCES
    ice/ice_auth_params.cpp
    ice/ice_transaction.cpp
    ice/ice_controller.cpp
    ice/ice_candidate.cpp
    ice/ice_endpoint.cpp
    ice/ice_server_params.cpp
    ice/ice_transport_params.cpp
    ice/ice_gathering_command.cpp
    ice/ice_gathering_state_event.cpp
    ice/ice_utils.cpp
    ice/ice_config.cpp
    ice/ice_transport_factory.cpp
    )

set(ICE_PUBLIC_HEADERS
    ice/ice_types.h
    ice/ice_auth_params.h
    ice/ice_candidate.h
    ice/ice_endpoint.h
    ice/ice_gathering_command.h
    ice/ice_gathering_state_event.h
    ice/ice_server_params.h
    ice/i_ice_transport.h
    )

set(ICE_PRIVATE_HEADERS
    ice/ice_utils.h
    ice/ice_transaction.h
    ice/ice_controller.h
    ice/ice_transport_params.h
    ice/ice_config.h
    ice/ice_transport_factory.h
    )

# tls sources
set(TLS_SOURCES
    tls/tls_config.cpp
    tls/tls_fingerprint.cpp
    tls/tls_endpoint.cpp
    tls/tls_transport_params.cpp
    tls/tls_transport_factory.cpp
    tls/tls_keys_event.cpp
    tls/tls_packet_impl.cpp
    )

set(TLS_PUBLIC_HEADERS
    tls/tls_types.h
    tls/tls_fingerprint.h
    tls/tls_endpoint.h
    tls/tls_config.h
    tls/tls_transport_params.h
    tls/tls_keys_event.h
    tls/i_tls_transport.h
    tls/i_tls_packet.h
    )

set(TLS_PRIVATE_HEADERS
    tls/tls_transport_factory.h
    tls/tls_packet_impl.h
    )

# serial sources
set(SERIAL_SOURCES
    serial/serial_endpoint.cpp
    serial/serial_packet_impl.cpp
    serial/serial_packet_builder_impl.cpp
    serial/serial_transport_factory.cpp
    serial/serial_transport_params.cpp
    )

set(SERIAL_PUBLIC_HEADERS
    serial/serial_types.h
    serial/serial_endpoint.h
    serial/i_serial_packet.h
    serial/i_serial_packet_builder.h
    serial/i_serial_transport.h
    )

set(SERIAL_PRIVATE_HEADERS
    serial/serial_packet_impl.h
    serial/serial_packet_builder_impl.h
    serial/serial_transport_factory.h
    serial/serial_transport_params.h
    )

# sq sources
set(SQ_SOURCES
    sq/mapped_sq_header.cpp
    sq/sq_packet_builder.cpp
    sq/sq_packet.cpp
    sq/sq_parser.cpp
    sq/sq_stitcher.cpp
    )

set(SQL_PUBLIC_HEADERS

    )

set(SQ_PRIVATE_HEADERS
    sq/sq_types.h
    sq/sq_packet_builder.h
    sq/sq_packet.h
    sq/sq_parser.h
    sq/sq_stitcher.h
    sq/mapped_sq_header.h
    )

set(NET_SOURCES
    endpoint.cpp
    enum_convert_impl.cpp
    enum_serialize_impl.cpp
    net_serialize_impl.cpp

    net_utils.cpp
    net_event_message_impl.cpp
    net_command_message_impl.cpp
    net_engine_impl.cpp
    net_engine_config.cpp
)

set(NET_PUBLIC_HEADERS
    net_types.h
    net_module_types.h
    net_event_types.h
    net_command_types.h
    endpoint.h
    i_net_packet.h
    i_net_module.h
    i_net_engine.h
    i_transport_channel.h
    i_transport_factory.h
    i_transport_collection.h
    i_net_packet.h
    i_net_packet_builder.h
    net_engine_config.h
    )

set(NET_PRIVATE_HEADERS
    net_utils.h
    net_engine_impl.h
)

set(SOURCES
    ${NET_SOURCES}
    ${SOCKET_SOURCES}
    ${STUN_SOURCES}
    ${ICE_SOURCES}
    ${TLS_SOURCES}
    ${SERIAL_SOURCES}
    ${SQ_SOURCES}
    )

set(PUBLIC_HEADERS
    ${NET_PUBLIC_HEADERS}
    ${SOCKET_PUBLIC_HEADERS}
    ${STUN_PUBLIC_HEADERS}
    ${ICE_PUBLIC_HEADERS}
    ${TLS_PUBLIC_HEADERS}
    ${SERIAL_PUBLIC_HEADERS}
    ${SQ_PUBLIC_HEADERS}
    )

set(PRIVATE_HEADERS
    ${NET_PRIVATE_HEADERS}
    ${SOCKET_PRIVATE_HEADERS}
    ${STUN_PRIVATE_HEADERS}
    ${ICE_PRIVATE_HEADERS}
    ${TLS_PRIVATE_HEADERS}
    ${SERIAL_PRIVATE_HEADERS}
    ${SQ_PRIVATE_HEADERS}
)


add_library(${TARGET}
    ${SOURCES}
    ${PRIVATE_HEADERS}
    ${PUBLIC_HEADERS}
)

install(FILES ${NET_PUBLIC_HEADERS}
        DESTINATION /home/user/test/net)


