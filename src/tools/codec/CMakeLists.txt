cmake_minimum_required(VERSION 3.16)

set(TARGET ptcodec)

set(SOURCES
    h264/h264_utils.cpp
    h264/h264_fragment.cpp
    h264/h264_mapped_headers.cpp
    h264/sps_header.cpp
)

set(PUBLIC_HEADERS
    h264/h264_types.h
    h264/h264_fragment.h
    h264/h264_utils.h
    h264/sps_header.h
)

set(PRIVATE_HEADERS
    h264/h264_mapped_headers.h
)

install(FILES ${PUBLIC_HEADERS}
        DESTINATION ${INSTALL_INCLUDE_DIR}/tools/codec)

add_library(${TARGET}
    ${SOURCES}
    ${PRIVATE_HEADERS}
    ${PUBLIC_HEADERS}
)
