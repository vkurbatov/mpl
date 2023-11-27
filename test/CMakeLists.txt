cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)

set(TEST_PROJECT_NAME
    "${PROJECT_NAME}_test"
    )

set(TEST_SOURCES
    main.cpp
    tests.cpp
)

set(TEST_HEADERS
    tests.h
)

add_executable(${TEST_PROJECT_NAME}
    ${TEST_SOURCES}
    ${TEST_HEADERS}
)

target_link_libraries(${TEST_PROJECT_NAME}
    ${PROJECT_NAME}
    )
    




