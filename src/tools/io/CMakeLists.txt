cmake_minimum_required(VERSION 3.16)

set(TARGET ptio)

# serial sources
set(SERIAL_SOURCES
    serial/serial_link_config.cpp
    serial/serial_endpoint.cpp
    serial/serial_link.cpp
    )

set(SERIAL_PUBLIC_HEADERS
    serial/serial_types.h
    serial/serial_link.h
    serial/serial_link_config.h
    serial/serial_endpoint.h
    )

set(SERIAL_PRIVATE_HEADERS
    )

install(FILES ${SERIAL_PUBLIC_HEADERS}
        DESTINATION ${INSTALL_INCLUDE_DIR}/tools/io/serial)

# net sources
set(NET_SOURCES
    net/ip_address.cpp
    net/ip_endpoint.cpp
    net/resolver.cpp
    net/net_info.cpp
    net/net_utils.cpp
    net/udp_link_config.cpp
    net/udp_link.cpp
    net/socket_options.cpp
    )

set(NET_PUBLIC_HEADERS
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

set(NET_PRIVATE_HEADERS
    )

install(FILES ${NET_PUBLIC_HEADERS}
        DESTINATION ${INSTALL_INCLUDE_DIR}/tools/io/net)

set(IO_SOURCES
    io_base.cpp
    io_core.cpp
    io_link_base.cpp
    io_worker_factory.cpp
    endpoint.cpp
)

set(IO_PUBLIC_HEADERS
    io_base.h
    i_io_link.h
    i_io_link_factory.h
    i_io_worker_factory.h
    endpoint.h
)

set(IO_PRIVATE_HEADERS
    io_core.h
    io_link_base.h
    io_worker_factory.h
    )

install(FILES ${IO_PUBLIC_HEADERS}
        DESTINATION ${INSTALL_INCLUDE_DIR}/tools/io)

set(SOURCES
    ${IO_SOURCES}
    ${SERIAL_SOURCES}
    ${NET_SOURCES}
    )

set(PUBLIC_HEADERS
    ${IO_PUBLIC_HEADERS}
    ${SERIAL_PUBLIC_HEADERS}
    ${NET_PUBLIC_HEADERS}
    )

set(PRIVATE_HEADERS
    ${IO_PRIVATE_HEADERS}
    ${SERIAL_PRIVATE_HEADERS}
    ${NET_PRIVATE_HEADERS}
)


add_library(${TARGET}
    ${SOURCES}
    ${PRIVATE_HEADERS}
    ${PUBLIC_HEADERS}
)
