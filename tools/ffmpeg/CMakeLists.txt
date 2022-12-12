cmake_minimum_required(VERSION 3.16)

set(TARGET avwrp)

set(SOURCES
    libav_base.cpp
    libav_converter.cpp
    libav_resampler.cpp
    libav_input_format.cpp
    libav_stream_grabber.cpp
    libav_stream_publisher.cpp
    libav_transcoder.cpp
    libav_utils.cpp
    test.cpp
)

set(PUBLIC_HEADERS
    libav_base.h
    libav_converter.h
    libav_resampler.h
    libav_input_format.h
    libav_stream_grabber.h
    libav_stream_publisher.h
    libav_transcoder.h
    libav_utils.h
    test.h
)

set(PRIVATE_HEADERS
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../base_utils)

add_library(${TARGET}
    ${SOURCES}
    ${PRIVATE_HEADERS}
    ${PUBLIC_HEADERS}
)
