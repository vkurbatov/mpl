cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)

set(SOURCES
    main.cpp
)

set(PUBLIC_HEADERS
)

set(PRIVATE_HEADERS
)

add_library(${PROJECT_NAME} SHARED
    ${SOURCES}
    ${PRIVATE_HEADERS}
    ${PUBLIC_HEADERS}
)

target_link_libraries(${PROJECT_NAME}
    mplapp
    mplmedia
    mplnet
    mplutils
    mpllog
    mplcore
    ptvnc
    ptvsc
    ptv4l2
    ptcv
    ptav
    ptipc
    ptwap
    ptio
    ptssl
    ptcodec
    ptutils
    vncclient
    ${WEBRTC_AUDIO_PROCESSING_LIBRARIES}
    ${OpenCV_LIBRARIES}
    ${OpenCV_LIBRARIES}
    ${AVCODEC_LIBRARIES}
    ${SWRESAMPLE_LIBRARIES}
    ${AVDEVICE_LIBRARIES}
    ${SWSCALE_LIBRARIES}
    ${AVUTIL_LIBRARIES}
    ${AVFORMAT_LIBRARIES}
    ${Boost_LIBRARIES}
    ${SRTP_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    )

install(TARGETS ${PROJECT_NAME}
        LIBRARY)


add_subdirectory(core)
add_subdirectory(utils)
add_subdirectory(media)
add_subdirectory(net)
add_subdirectory(app)
add_subdirectory(log)
add_subdirectory(tools)


    




