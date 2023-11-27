cmake_minimum_required(VERSION 3.16)

set(TARGET mplapp)

set(APP_SOURCES
    app_config.cpp
    event_message_factory_impl.cpp
    command_message_factory_impl.cpp
    app_engine_impl.cpp
)

set(APP_PUBLIC_HEADERS
    app_config.h
    app_module_types.h
    i_app_module.h
    i_app_engine.h
    )

set(APP_PRIVATE_HEADERS
    event_message_factory_impl.h
    command_message_factory_impl.h
    app_engine_impl.h
)

set(SOURCES
    ${APP_SOURCES}
    )

set(PUBLIC_HEADERS
    ${APP_PUBLIC_HEADERS}
    )

set(PRIVATE_HEADERS
    ${APP_PRIVATE_HEADERS}
)

install(FILES ${APP_PUBLIC_HEADERS}
        DESTINATION ${INSTALL_INCLUDE_DIR}/app)

add_library(${TARGET}
    ${SOURCES}
    ${PRIVATE_HEADERS}
    ${PUBLIC_HEADERS}
)

