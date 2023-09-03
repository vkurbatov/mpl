cmake_minimum_required(VERSION 3.16)

set(TARGET iopt)

set(SOURCES
    io_base.cpp
    io_core.cpp
    io_link_base.cpp
    serial/serial_link_config.cpp
    serial/serial_endpoint.cpp
    serial/serial_link.cpp
)

set(PUBLIC_HEADERS
    io_base.h
    i_io_link.h
    i_io_link_factory.h
    serial/serial_link_config.h
    serial/serial_endpoint.h
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
