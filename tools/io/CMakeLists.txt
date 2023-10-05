cmake_minimum_required(VERSION 3.16)

set(TARGET iopt)

set(SOURCES
    io_base.cpp
    io_core.cpp
    io_link_base.cpp
    endpoint.cpp
    serial/serial_link_config.cpp
    serial/serial_endpoint.cpp
    serial/serial_link.cpp
    net/ip_address.cpp
    net/ip_endpoint.cpp
    net/resolver.cpp
    net/net_info.cpp
    net/net_utils.cpp
    net/udp_link_config.cpp
    net/udp_link.cpp
    net/socket_options.cpp
)

set(PUBLIC_HEADERS
    io_base.h
    i_io_link.h
    i_io_link_factory.h
    endpoint.h
    serial/serial_link_config.h
    serial/serial_endpoint.h
    net/ip_address.h
    net/net_types.h
    net/ip_endpoint.h
    net/resolver.h
    net/net_info.h
    net/net_utils.h
    net/udp_link_config.h
    net/udp_link.h
    net/socket_options.h
)

set(PRIVATE_HEADERS
    io_core.h
    io_link_base.h
    serial/serial_types.h
    serial/serial_link.h
    )


add_library(${TARGET}
    ${SOURCES}
    ${PRIVATE_HEADERS}
    ${PUBLIC_HEADERS}
)
