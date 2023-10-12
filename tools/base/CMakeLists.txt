cmake_minimum_required(VERSION 3.16)

set(TARGET ptbase)

set(SOURCES
    system.cpp
    filesystem.cpp
    frame_base.cpp
    option_base.cpp
    string_base.cpp
    endian_base.cpp
    time_base.cpp
    variant.cpp
    sync_base.cpp
    data_container.cpp
    bitstream_base.cpp
    random_base.cpp
    convert_base.cpp
    any_base.cpp
    url_base.cpp
)

set(PUBLIC_HEADERS
    system.h
    filesystem.h
    frame_base.h
    option_base.h
    string_base.h
    endian_base.h
    time_base.h
    variant.h
    logger_base.h
    data_container.h
    enum_convert_defs.h
    sync_base.h
    enum_utils.h
    helper_defs.h
    bitstream_base.h
    random_base.h
    convert_base.h
    any_base.h
    url_base.h
)

set(PRIVATE_HEADERS
)

add_library(${TARGET}
    ${SOURCES}
    ${PRIVATE_HEADERS}
    ${PUBLIC_HEADERS}
)
