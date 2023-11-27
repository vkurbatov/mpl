cmake_minimum_required(VERSION 3.16)

set(TARGET mplmedia)

# ipc sources
set(IPC_SOURCES
    ipc/ipc_input_device_factory.cpp
    ipc/ipc_output_device_factory.cpp
    ipc/ipc_input_device_params.cpp
    ipc/ipc_output_device_params.cpp
    )

set(IPC_PUBLIC_HEADERS
    )

set(IPC_PRIVATE_HEADERS
    ipc/ipc_input_device_factory.h
    ipc/ipc_output_device_factory.h
    ipc/ipc_input_device_params.h
    ipc/ipc_output_device_params.h
    )

# libav sources
set(LIBAV_SOURCES
    libav/libav_video_converter_factory.cpp
    libav/libav_audio_converter_factory.cpp
    libav/libav_transcoder_factory.cpp
    libav/libav_input_device_factory.cpp
    libav/libav_output_device_factory.cpp
    libav/libav_format_utils.cpp
    )

set(LIBAV_PUBLIC_HEADERS
    )

set(LIBAV_PRIVATE_HEADERS
    libav/libav_format_utils.h
    libav/libav_video_converter_factory.h
    libav/libav_audio_converter_factory.h
    libav/libav_transcoder_factory.h
    libav/libav_input_device_factory.h
    libav/libav_output_device_factory.h
    )

# vnc sources
set(VNC_SOURCES
    vnc/vnc_device_factory.cpp
    )

set(VNC_PUBLIC_HEADERS
    )

set(VNC_PRIVATE_HEADERS
    vnc/vnc_device_factory.h
    )

# apm sources
set(APM_SOURCES
    apm/apm_device_factory.cpp
    apm/apm_device_params.cpp
    )

set(APM_PUBLIC_HEADERS
    )

set(APM_PRIVATE_HEADERS
    apm/apm_device_factory.h
    apm/apm_device_params.h
    )

# visca sources
set(VISCA_SOURCES
    visca/visca_device_factory.cpp
    )

set(VISCA_PUBLIC_HEADERS
    )

set(VISCA_PRIVATE_HEADERS
    visca/visca_device_factory.h
    )

# v4l2 sources
set(V4L2_SOURCES
    v4l2/v4l2_device_factory.cpp
    v4l2/v4l2_utils.cpp
    )

set(V4L2_PUBLIC_HEADERS
    )

set(V4L2_PRIVATE_HEADERS
    v4l2/v4l2_device_factory.h
    v4l2/v4l2_utils.h
    )

# mcu sources
set(MCU_SOURCES
    mcu/compose_audio_track_params.cpp
    mcu/compose_video_track_params.cpp
    mcu/compose_stream_params.cpp
    mcu/audio_composer.cpp
    mcu/video_composer.cpp
    mcu/layout_manager_mosaic_impl.cpp
    mcu/media_composer_factory_impl.cpp
    )

set(MCU_PUBLIC_HEADERS
    )

set(MCU_PRIVATE_HEADERS
    mcu/compose_audio_track_params.h
    mcu/compose_video_track_params.h
    mcu/compose_stream_params.h
    mcu/audio_composer.h
    mcu/video_composer.h
    mcu/layout_manager_mosaic_impl.h
    mcu/media_composer_factory_impl.h
    )
# media sources
set(MEDIA_SOURCES
    enum_convert_impl.cpp
    enum_serialize_impl.cpp
    media_utils.cpp
    format_utils.cpp
    device_info.cpp
    audio_frame_splitter.cpp
    audio_format_impl.cpp
    audio_format_info.cpp
    audio_frame_impl.cpp
    audio_format_helper.cpp
    audio_mixer.cpp
    audio_level.cpp
    audio_info.cpp
    audio_sample.cpp
    video_format_impl.cpp
    video_format_info.cpp
    video_frame_impl.cpp
    video_info.cpp
    media_info.cpp
    media_frame_info.cpp
    media_packetizer_impl.cpp
    timestamp_calculator.cpp
    track_info.cpp
    track_metrics.cpp
    track_metrics_manager.cpp
    track_params.cpp
    frame_option_container.cpp
    media_converter_factory_impl.cpp
    media_serialize_impl.cpp
    smart_transcoder_factory.cpp
    draw_options.cpp
    image_frame.cpp
    image_builder.cpp
    command_camera_control.cpp
    media_command_message_impl.cpp
    media_frame_buffer.cpp
    media_frame_selector.cpp
    media_sink_impl.cpp
    media_engine_config.cpp
    media_engine_impl.cpp
    media_format_factory_impl.cpp
    media_frame_builder_impl.cpp
)

set(MEDIA_PUBLIC_HEADERS
    media_types.h
    media_event_types.h
    media_command_types.h
    media_module_types.h
    device_types.h
    device_info.h
    audio_types.h
    audio_format_info.h
    audio_info.h
    video_types.h
    video_frame_types.h
    video_format_info.h
    video_info.h
    media_info.h
    stream_types.h
    media_option_types.h
    media_frame_info.h
    command_camera_control.h
    track_info.h
    track_metrics.h
    track_params.h
    i_media_stream.h
    i_media_track.h
    i_media_format.h
    i_media_format_factory.h
    i_media_format_collection.h
    i_message_media_data.h
    i_media_frame.h
    i_media_converter.h
    i_media_converter_factory.h
    i_media_converter_collection.h
    i_layout.h
    i_layout_manager.h
    i_compose_stream.h
    i_media_composer_factory.h
    i_media_composer.h
    i_audio_format.h
    i_video_format.h
    i_audio_frame.h
    i_video_frame.h
    i_device.h
    i_device_factory.h
    i_device_factory_collection.h
    i_frame_buffer.h
    i_media_module.h
    i_media_engine.h
    i_media_frame_builder.h
    )

set(MEDIA_PRIVATE_HEADERS
    media_utils.h
    format_utils.h
    audio_frame_splitter.h
    audio_format_impl.h
    video_format_impl.h
    audio_frame_impl.h
    video_frame_impl.h
    audio_format_helper.h
    audio_mixer.h
    audio_level.h
    audio_sample.h
    timestamp_calculator.h
    track_metrics_manager.h
    frame_option_container.h
    media_converter_factory_impl.h
    smart_transcoder_factory.h
    draw_options.h
    image_frame.h
    image_builder.h
    media_frame_buffer.h
    media_frame_selector.h
    media_sink_impl.h
    media_engine_config.h
    media_engine_impl.h
    media_format_factory_impl.h
    media_frame_builder_impl.h
)

install(FILES ${MEDIA_PUBLIC_HEADERS}
        DESTINATION ${INSTALL_INCLUDE_DIR}/media)

set(SOURCES
    ${MEDIA_SOURCES}
    ${IPC_SOURCES}
    ${LIBAV_SOURCES}
    ${VNC_SOURCES}
    ${V4L2_SOURCES}
    ${APM_SOURCES}
    ${VISCA_SOURCES}
    ${MCU_SOURCES}
    )
set(PUBLIC_HEADERS
    ${MEDIA_PUBLIC_HEADERS}
    ${IPC_PUBLIC_HEADERS}
    ${LIBAV_PUBLIC_HEADERS}
    ${VNC_PUBLIC_HEADERS}
    ${V4L2_PUBLIC_HEADERS}
    ${APM_PUBLIC_HEADERS}
    ${VISCA_PUBLIC_HEADERS}
    ${MCU_PUBLIC_HEADERS}
    )
set(PRIVATE_HEADERS
    ${MEDIA_PRIVATE_HEADERS}
    ${IPC_PRIVATE_HEADERS}
    ${LIBAV_PRIVATE_HEADERS}
    ${VNC_PRIVATE_HEADERS}
    ${V4L2_PRIVATE_HEADERS}
    ${APM_PRIVATE_HEADERS}
    ${VISCA_PRIVATE_HEADERS}
    ${MCU_PRIVATE_HEADERS}
    )

add_library(${TARGET}
    ${SOURCES}
    ${PRIVATE_HEADERS}
    ${PUBLIC_HEADERS}
)
