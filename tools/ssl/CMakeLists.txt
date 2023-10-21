cmake_minimum_required(VERSION 3.16)

set(TARGET sslpt)

set(SOURCES
    const_ssl_message.cpp
    dynamic_ssl_message.cpp
    custom_ssl_connection_observer.cpp
    mapped_dtls_header.cpp
    ssl_context.cpp
    ssl_context_config.cpp
    ssl_adapter.cpp
    ssl_ec_key.cpp
    ssl_private_key.cpp
    ssl_x509.cpp
    ssl_connection_config.cpp
    ssl_utils.cpp
    ssl_manager.cpp
    ssl_manager_config.cpp
    bio_handler.cpp
    bio_context.cpp
    bio_config.cpp
    srtp_session.cpp
    srtp_session_config.cpp
    srtp_packet.cpp
    srtp_key_info.cpp

    ssl_session_params.cpp
    ssl_certificate_impl.cpp
    ssl_session_factory.cpp
    test.cpp
)

set(PUBLIC_HEADERS
    i_ssl_channel.h
    i_ssl_message.h
    i_ssl_message_sink.h
    i_ssl_connection.h
    i_ssl_connection_observer.h
    i_ssl_session.h
    i_ssl_certificate.h
    ssl_pointers.h
    ssl_context.h
    ssl_context_config.h
    ssl_adapter.h
    ssl_ec_key.h
    ssl_private_key.h
    ssl_x509.h
    ssl_connection_config.h
    bio_handler.h
    bio_context.h
    bio_config.h
    ssl_manager.h
    ssl_manager_config.h
    ssl_utils.h
    hash_types.h
    ssl_types.h
    srtp_types.h
    srtp_session.h
    srtp_session_config.h
    srtp_packet.h
    srtp_key_info.h

    ssl_session_params.h
    ssl_certificate_impl.h
    ssl_session_factory.h
    test.h
)

set(PRIVATE_HEADERS
    dynamic_ssl_message.h
    const_ssl_message.h
    custom_ssl_connection_observer.h
    mapped_dtls_header.h
    ssl_native.h
)


add_library(${TARGET}
    ${SOURCES}
    ${PRIVATE_HEADERS}
    ${PUBLIC_HEADERS}
)
