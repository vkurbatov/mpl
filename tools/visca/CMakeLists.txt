cmake_minimum_required(VERSION 3.16)

set(TARGET ptvsc)

set(SOURCES
    visca_proto.cpp
    visca_base.cpp
    visca_control.cpp
    visca_test.cpp
)

set(PUBLIC_HEADERS
    i_visca_channel.h
)

set(PRIVATE_HEADERS
    visca_proto.h
    visca_base.h
    visca_control.h
    visca_test.h
)


add_library(${TARGET}
    ${SOURCES}
    ${PRIVATE_HEADERS}
    ${PUBLIC_HEADERS}
)
