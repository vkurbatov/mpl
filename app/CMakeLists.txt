cmake_minimum_required(VERSION 3.16)

set(TARGET mplapp)

set(SOURCES
    app_config.cpp
    event_message_factory_impl.cpp
    command_message_factory_impl.cpp
    app_engine_impl.cpp
)

set(PUBLIC_HEADERS
    app_config.h
    app_module_types.h
    i_app_module.h
    i_app_engine.h
    )

set(PRIVATE_HEADERS
    event_message_factory_impl.h
    command_message_factory_impl.h
    app_engine_impl.h
)


add_library(${TARGET}
    ${SOURCES}
    ${PRIVATE_HEADERS}
    ${PUBLIC_HEADERS}
)

