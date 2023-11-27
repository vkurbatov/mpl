cmake_minimum_required(VERSION 3.16)

set(TARGET ptipc)

set(SOURCES
    ipc_base.cpp
    ipc_manager.cpp
    ipc_segment.cpp
)

set(PUBLIC_HEADERS
    ipc_base.h
    ipc_types.h
    ipc_manager.h
    ipc_segment.h
)

set(PRIVATE_HEADERS
)

install(FILES ${PUBLIC_HEADERS}
        DESTINATION ${INSTALL_INCLUDE_DIR}/tools/ipc)

add_library(${TARGET}
    ${SOURCES}
    ${PRIVATE_HEADERS}
    ${PUBLIC_HEADERS}
)
