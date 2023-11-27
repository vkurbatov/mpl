cmake_minimum_required(VERSION 3.16)

set(TARGET mpllog)

set(LOG_SOURCES
    log_message.cpp
    logger_config.cpp
    logger_impl.cpp
    log_utils.cpp
    log_tools.cpp
    enum_convert_impl.cpp
    )

set(LOG_PUBLIC_HEADERS
    log_types.h
    log_message.h
    i_logger.h
    )

set(LOG_PRIVATE_HEADERS
    logger_impl.h
    logger_config.h
    log_utils.h
    log_tools.h
)


set(SOURCES
    ${LOG_SOURCES}
    )

set(PUBLIC_HEADERS
    ${LOG_PUBLIC_HEADERS}
    )

set(PRIVATE_HEADERS
    ${LOG_PRIVATE_HEADERS}
)

install(FILES ${LOG_PUBLIC_HEADERS}
        DESTINATION ${INSTALL_INCLUDE_DIR}/log)

add_library(${TARGET}
    ${SOURCES}
    ${PRIVATE_HEADERS}
    ${PUBLIC_HEADERS}
)

