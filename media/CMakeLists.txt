cmake_minimum_required(VERSION 3.16)

set(TARGET media)

set(SOURCES
    test.cpp
    smart_buffer.cpp
    stream_info.cpp
    utils.cpp
    time_utils.cpp
    media_utils.cpp
    property_helper.cpp
    property_reader.cpp
    property_writer.cpp
    string_utils_impl.cpp
    property_value_impl.cpp
    property_tree_impl.cpp
    option_impl.cpp
    option_helper.cpp
    convert_value_impl.cpp
    convert_property_impl.cpp
    audio_format_impl.cpp
    audio_info.cpp
    video_format_impl.cpp
    video_info.cpp
)

set(PUBLIC_HEADERS
    common_types.h
    media_types.h
    audio_types.h
    audio_info.h
    video_types.h
    video_info.h
    stream_info.h
    property_types.h
    time_types.h
    i_property.h
    i_property_value.h
    i_property_tree.h
    i_option.h
    i_data_object.h
    i_buffer.h
    i_dynamic_buffer.h
    i_media_format.h
    i_media_frame.h
    i_audio_format.h
    i_video_format.h
    )

set(PRIVATE_HEADERS
    test.h
    smart_buffer.h
    utils.h
    time_utils.h
    media_utils.h
    property_helper.h
    property_reader.h
    property_writer.h
    convert_utils.h
    property_value_impl.h
    property_tree_impl.h
    option_impl.h
    option_helper.h
    audio_format_impl.h
    video_format_impl.h
)


add_library(${TARGET}
    ${SOURCES}
    ${PRIVATE_HEADERS}
    ${PUBLIC_HEADERS}
)
