cmake_minimum_required(VERSION 3.16)

set(TARGET media)

set(SOURCES
    test.cpp
)

set(PUBLIC_HEADERS
)

set(PRIVATE_HEADERS
    test.h
)


add_library(${TARGET}
    ${SOURCES}
    ${PRIVATE_HEADERS}
    ${PUBLIC_HEADERS}
)
