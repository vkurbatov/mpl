cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)

project(mpl)


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

set(INSTALL_INCLUDE_DIR "include/${PROJECT_NAME}")

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
    ${WEBRTC_AUDIO_PROCESSING_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src
    )
    
link_directories(
    ${Boost_LIBRARY_DIRS}
    ${OpenCV_LIBRARY_DIRS}
    ${WEBRTC_AUDIO_PROCESSING_LIBRARY_DIRS}
    )

configure_file(pc/mpl.pc.in mpl.pc @ONLY)

install(FILES ${CMAKE_BINARY_DIR}/mpl.pc DESTINATION lib/pkgconfig)

add_subdirectory(src)
add_subdirectory(test)




    




