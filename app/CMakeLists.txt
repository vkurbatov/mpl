cmake_minimum_required(VERSION 3.16)

set(TARGET mplapp)

set(SOURCES
    app_config.cpp
    test.cpp
)

set(PUBLIC_HEADERS
    app_config.h
    i_app_engine.h
    i_core_module.h
    )

set(PRIVATE_HEADERS
    test.h
)


add_library(${TARGET}
    ${SOURCES}
    ${PRIVATE_HEADERS}
    ${PUBLIC_HEADERS}
)

