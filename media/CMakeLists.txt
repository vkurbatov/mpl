cmake_minimum_required(VERSION 3.16)

set(TARGET media)

set(SOURCES
    test.cpp
    smart_buffer.cpp
    stream_info.cpp
    utils.cpp
    string_utils_impl.cpp
    property_value_impl.cpp
    property_tree_impl.cpp
    convert_property_impl.cpp
)

set(PUBLIC_HEADERS
    common_types.h
    media_types.h
    stream_info.h
    property_types.h
    i_property.h
    i_property_value.h
    i_property_tree.h
    i_data_object.h
    i_buffer.h
    i_dynamic_buffer.h
    i_media_format.h
    i_media_frame.h

    )

set(PRIVATE_HEADERS
    test.h
    smart_buffer.h
    utils.h
    convert.h
    property_value_impl.h
    property_tree_impl.h
)


add_library(${TARGET}
    ${SOURCES}
    ${PRIVATE_HEADERS}
    ${PUBLIC_HEADERS}
)
