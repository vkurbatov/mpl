cmake_minimum_required(VERSION 3.16)

set(TARGET ptvnc)

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

install(FILES ${PUBLIC_HEADERS}
        DESTINATION ${INSTALL_INCLUDE_DIR}/tools/vnc)

add_library(${TARGET}
    ${SOURCES}
    ${PRIVATE_HEADERS}
    ${PUBLIC_HEADERS}
)
