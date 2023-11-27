cmake_minimum_required(VERSION 3.16)

set(TARGET ptwap)

set(SOURCES
    wap_base.cpp
    wap_processor.cpp
)

set(PUBLIC_HEADERS
    wap_base.h
    wap_processor.h
)

set(PRIVATE_HEADERS
)

install(FILES ${PUBLIC_HEADERS}
        DESTINATION ${INSTALL_INCLUDE_DIR}/tools/wap)

add_library(${TARGET}
    ${SOURCES}
    ${PRIVATE_HEADERS}
    ${PUBLIC_HEADERS}
)
