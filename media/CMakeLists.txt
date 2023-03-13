cmake_minimum_required(VERSION 3.16)

set(TARGET mplmedia)

set(SOURCES
    test.cpp
    enum_convert_impl.cpp
    enum_serialize_impl.cpp
    stream_info.cpp
    media_utils.cpp
    format_utils.cpp
    libav_format_utils.cpp
    audio_frame_splitter.cpp
    audio_format_impl.cpp
    audio_info.cpp
    audio_frame_impl.cpp
    audio_format_helper.cpp
    video_format_impl.cpp
    video_info.cpp
    video_frame_impl.cpp
    message_frame_impl.cpp
    libav_video_converter_factory.cpp
    libav_audio_converter_factory.cpp
    libav_transcoder_factory.cpp
    libav_input_device_factory.cpp
    libav_output_device_factory.cpp
    media_converter_factory_impl.cpp
    media_serialize_impl.cpp
    smart_transcoder_factory.cpp
    v4l2_device_factory.cpp
    v4l2_utils.cpp
)

set(PUBLIC_HEADERS
    media_types.h
    device_types.h
    audio_types.h
    audio_info.h
    video_types.h
    video_info.h
    stream_info.h
    stream_types.h
    media_option_types.h
    i_message_frame.h
    i_media_stream.h
    i_media_format.h
    i_media_frame.h
    i_media_converter_factory.h
    i_media_converter.h
    i_audio_format.h
    i_video_format.h
    i_audio_frame.h
    i_video_frame.h
    i_device.h
    i_device_factory.h
    )

set(PRIVATE_HEADERS
    test.h
    media_utils.h
    libav_format_utils.h
    format_utils.h
    audio_frame_splitter.h
    audio_format_impl.h
    video_format_impl.h
    audio_frame_impl.h
    video_frame_impl.h
    audio_format_helper.h
    message_frame_impl.h
    libav_video_converter_factory.h
    libav_audio_converter_factory.h
    libav_transcoder_factory.h
    libav_input_device_factory.h
    libav_output_device_factory.h
    media_converter_factory_impl.h
    smart_transcoder_factory.h
    v4l2_device_factory.h
    v4l2_utils.h
)


add_library(${TARGET}
    ${SOURCES}
    ${PRIVATE_HEADERS}
    ${PUBLIC_HEADERS}
)
