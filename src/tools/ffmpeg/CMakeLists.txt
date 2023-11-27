cmake_minimum_required(VERSION 3.16)

set(TARGET ptav)

set(SOURCES
    libav_base.cpp
    libav_converter.cpp
    libav_resampler.cpp
    libav_input_format.cpp
    libav_output_format.cpp
    libav_transcoder.cpp
    libav_utils.cpp
)

set(PUBLIC_HEADERS
    libav_base.h
    libav_converter.h
    libav_resampler.h
    libav_input_format.h
    libav_output_format.h
    libav_transcoder.h
    libav_utils.h
)

set(PRIVATE_HEADERS
)

install(FILES ${PUBLIC_HEADERS}
        DESTINATION ${INSTALL_INCLUDE_DIR}/tools/ffmpeg)


add_library(${TARGET}
    ${SOURCES}
    ${PRIVATE_HEADERS}
    ${PUBLIC_HEADERS}
)
