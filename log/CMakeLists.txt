cmake_minimum_required(VERSION 3.16)

set(TARGET mpllog)

set(SOURCES
    log_message.cpp
    logger_config.cpp
    logger_impl.cpp
    log_utils.cpp
    log_tools.cpp
    enum_convert_impl.cpp
    )

set(PUBLIC_HEADERS
    log_types.h
    log_message.h
    i_logger.h
    )

set(PRIVATE_HEADERS
    logger_impl.h
    logger_config.h
    log_utils.h
    log_tools.h
)


add_library(${TARGET}
    ${SOURCES}
    ${PRIVATE_HEADERS}
    ${PUBLIC_HEADERS}
)

