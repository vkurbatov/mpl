cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)

project(mpl)
set(TEST_PROJECT_NAME
    "${PROJECT_NAME}_test"
    )

find_package(PkgConfig REQUIRED)

pkg_search_module(AVCODEC REQUIRED libavcodec)
pkg_search_module(AVFORMAT REQUIRED libavformat)
pkg_search_module(AVDEVICE REQUIRED libavdevice)
pkg_search_module(SWRESAMPLE REQUIRED libswresample)
pkg_search_module(AVUTIL REQUIRED libavutil)
pkg_search_module(SWSCALE REQUIRED libswscale)
pkg_search_module(V4L2 QUIET libv4l2>=0.8.8)

set(OpenCV_STATIC ON)
find_package(OpenCV REQUIRED COMPONENTS core imgproc freetype)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS system filesystem)

string(LENGTH "${CMAKE_SOURCE_DIR}/" SOURCE_PATH_SIZE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -W -Wall -Wno-unused-parameter -Werror=return-type -Werror=reorder -Wno-switch -Wno-deprecated-declarations")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSOURCE_PATH_SIZE=${SOURCE_PATH_SIZE}")

add_definitions("-DSOURCE_PATH_SIZE=${SOURCE_PATH_SIZE}")

set(BOOST_NO_RTTI yes)

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
    )
    
link_directories(
    ${Boost_LIBRARY_DIRS}
    ${OpenCV_LIBRARY_DIRS}
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

target_link_libraries(${PROJECT_NAME}
    media
    vncwrp
    viscawrp
    v4l2wrp
    srlwrp
    ocvwrp
    avwrp
    base
    )

set(TEST_SOURCES
    test/main.cpp
)

set(TEST_PUBLIC_HEADERS
)

set(TEST_PRIVATE_HEADERS
)


add_executable(${TEST_PROJECT_NAME}
    ${SOURCES}
    ${PRIVATE_HEADERS}
    ${PUBLIC_HEADERS}
)

target_link_libraries(${TEST_PROJECT_NAME}
    ${PROJECT_NAME}
    )

add_subdirectory(media)
add_subdirectory(tools)


    




