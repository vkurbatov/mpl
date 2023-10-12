cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)

project(mpl)
set(TEST_PROJECT_NAME
    "${PROJECT_NAME}_test"
    )

set(IPC_SRV_PROJECT_NAME
    "ipc_srv")

set(IPC_CLI_PROJECT_NAME
    "ipc_cli")

find_package(PkgConfig REQUIRED)

pkg_search_module(AVCODEC REQUIRED libavcodec)
pkg_search_module(AVFORMAT REQUIRED libavformat)
pkg_search_module(AVDEVICE REQUIRED libavdevice)
pkg_search_module(SWRESAMPLE REQUIRED libswresample)
pkg_search_module(AVUTIL REQUIRED libavutil)
pkg_search_module(SWSCALE REQUIRED libswscale)
pkg_search_module(WEBRTC_AUDIO_PROCESSING REQUIRED webrtc-audio-processing >=0.3-1)
pkg_search_module(V4L2 QUIET libv4l2>=0.8.8)
pkg_search_module(OPENSSL REQUIRED openssl)
pkg_search_module(SRTP REQUIRED libsrtp2)

add_definitions (${WEBRTC_AUDIO_PROCESSING_CFLAGS})


set(OpenCV_STATIC ON)
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs freetype)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS system filesystem)

string(LENGTH "${CMAKE_SOURCE_DIR}/" SOURCE_PATH_SIZE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -W -Wall -Wno-unused-parameter -Werror=return-type -Werror=reorder -Wno-switch -Wno-deprecated-declarations")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLINUX -DSOURCE_PATH_SIZE=${SOURCE_PATH_SIZE}")

add_definitions("-DSOURCE_PATH_SIZE=${SOURCE_PATH_SIZE}")

set(BOOST_NO_RTTI yes)

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
    ${WEBRTC_AUDIO_PROCESSING_INCLUDE_DIRS}
    )
    
link_directories(
    ${Boost_LIBRARY_DIRS}
    ${OpenCV_LIBRARY_DIRS}
    ${WEBRTC_AUDIO_PROCESSING_LIBRARY_DIRS}
    )

set(SOURCES
    main.cpp
)

set(PUBLIC_HEADERS
)

set(PRIVATE_HEADERS
)

add_library(${PROJECT_NAME}
    ${SOURCES}
    ${PRIVATE_HEADERS}
    ${PUBLIC_HEADERS}
)

message("Boost_LIBRARIES: ${Boost_LIBRARIES}")

target_link_libraries(${PROJECT_NAME}
    mplmedia
    mplnet
    mplutils
    mplcore
    vncpt
    viscapt
    v4l2pt
    srlpt
    ocvpt
    avpt
    ipcpt
    wappt
    iopt
    sslpt
    ptbase
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

set(TEST_SOURCES
    test/main.cpp
)


add_executable(${TEST_PROJECT_NAME}
    ${TEST_SOURCES}
)

target_link_libraries(${TEST_PROJECT_NAME}
    ${PROJECT_NAME}
    )

set(IPC_SRV_SOURCES
    ipc_srv/main.cpp)

add_executable(${IPC_SRV_PROJECT_NAME}
    ${IPC_SRV_SOURCES}
)

target_link_libraries(${IPC_SRV_PROJECT_NAME}
    ${PROJECT_NAME}
    )

set(IPC_CLI_SOURCES
    ipc_cli/main.cpp)

add_executable(${IPC_CLI_PROJECT_NAME}
    ${IPC_CLI_SOURCES}
)

target_link_libraries(${IPC_CLI_PROJECT_NAME}
    ${PROJECT_NAME}
    )

add_subdirectory(core)
add_subdirectory(utils)
add_subdirectory(media)
add_subdirectory(net)
add_subdirectory(tools)


    




