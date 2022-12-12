cmake_minimum_required(VERSION 3.16)

set(TARGET srlwrp)

set(SOURCES
    serial_api.cpp
    serial_base.cpp
    serial_device.cpp
)

set(PUBLIC_HEADERS
)

set(PRIVATE_HEADERS
    serial_api.h
    serial_base.h
    serial_device.h
)


add_library(${TARGET}
    ${SOURCES}
    ${PRIVATE_HEADERS}
    ${PUBLIC_HEADERS}
)
