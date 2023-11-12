cmake_minimum_required(VERSION 3.16)

set(TARGET ptv4l2)

set(SOURCES
    v4l2_api.cpp
    v4l2_utils.cpp
    v4l2_base.cpp
    v4l2_input_device.cpp
)

set(PUBLIC_HEADERS
)

set(PRIVATE_HEADERS
    v4l2_api.h
    v4l2_utils.h
    v4l2_base.h
    v4l2_input_device.h
)


add_library(${TARGET}
    ${SOURCES}
    ${PRIVATE_HEADERS}
    ${PUBLIC_HEADERS}
)
