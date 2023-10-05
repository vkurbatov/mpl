cmake_minimum_required(VERSION 3.16)

set(TARGET ipcpt)

set(SOURCES
    ipc_base.cpp
    ipc_manager.cpp
    ipc_segment.cpp
    test.cpp
)

set(PUBLIC_HEADERS
)

set(PRIVATE_HEADERS
    ipc_base.h
    ipc_types.h
    ipc_manager.h
    ipc_segment.h
    test.h
)


add_library(${TARGET}
    ${SOURCES}
    ${PRIVATE_HEADERS}
    ${PUBLIC_HEADERS}
)
