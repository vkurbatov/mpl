cmake_minimum_required(VERSION 3.16)

set(TARGET viscawrp)

set(SOURCES
    visca_proto.cpp
    visca_base.cpp
    visca_device.cpp
    visca_test.cpp
)

set(PUBLIC_HEADERS
)

set(PRIVATE_HEADERS
    visca_proto.h
    visca_base.h
    visca_device.h
    visca_test.h
)


add_library(${TARGET}
    ${SOURCES}
    ${PRIVATE_HEADERS}
    ${PUBLIC_HEADERS}
)
