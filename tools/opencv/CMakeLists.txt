cmake_minimum_required(VERSION 3.16)

set(TARGET ocvwrp)

set(SOURCES
    ocv_utils.cpp
    frame_info.cpp
    font_format.cpp
    draw_format.cpp
    draw_processor.cpp
    test.cpp
)

set(PUBLIC_HEADERS
    ocv_types.h
    ocv_utils.h
    frame_info.h
    font_format.h
    draw_format.h
    draw_processor.h
    test.h
)

set(PRIVATE_HEADERS
)


add_library(${TARGET}
    ${SOURCES}
    ${PRIVATE_HEADERS}
    ${PUBLIC_HEADERS}
)
