cmake_minimum_required(VERSION 3.16)

set(TARGET vncpt)

set(SOURCES
    vnc_base.cpp
    vnc_device.cpp
)

set(PUBLIC_HEADERS
)

set(PRIVATE_HEADERS
    vnc_base.h
    vnc_device.h
)


add_library(${TARGET}
    ${SOURCES}
    ${PRIVATE_HEADERS}
    ${PUBLIC_HEADERS}
)
