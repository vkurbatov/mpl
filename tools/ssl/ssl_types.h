#ifndef SSL_TYPES_H
#define SSL_TYPES_H

#include "hash_types.h"
#include <cstdint>

namespace ssl
{

enum class ssl_version_t
{
    undefined = -1,
    ssl3 = 0x0300,
    tls1 = 0x0301,
    tls1_1 = 0x0302,
    tls1_2 = 0x0303,
    tls1_3 = 0x0304,
    dtls1 = 0xfeff,
    dtls1_2 = 0xfefd
};

enum class ssl_content_type_t
{
    undefined = 0,
    change_cipher_spec = 20,
    alert = 21,
    handshake = 22,
    application_data = 23
};

enum class ssl_method_t
{
    default_method,
    tls_method,
    dtls_method,
};

enum class ssl_handshake_type_t
{
    undefined = -1,
    hello_request = 0,
    client_hello = 1,
    server_hello = 2,
    hello_verify_request = 3,
    new_session_ticket = 4,
    end_of_early_data = 5,
    hello_retry_request = 6,
    encrypted_extensions = 8,
    certificate = 11,
    server_key_exchange = 12,
    certificate_request = 13,
    server_hello_done = 14,
    certificate_verify = 15,
    client_key_exchange = 16,
    finished = 20,
    key_update = 24,
    message_hash = 254
};

enum class ssl_options_flags_t : std::uint32_t
{
    none = 0,
    no_query_mtu = 0x00001000,
    no_ticket = 0x00004000,
    cipter_server_preference = 0x00400000,
    no_tls_1 = 0x04000000,
    no_tls_1_2 = 0x08000000,
    no_tls_1_1 = 0x10000000,
    no_tls_1_3 = 0x20000000,
    no_dtls_1 = no_tls_1,
    no_dtls_1_2 = no_tls_1_2,
    default_options = cipter_server_preference | no_ticket | no_query_mtu
};

enum class ssl_session_cache_flags_t : std::uint32_t
{
    none = 0x0000,
    off = 0x0000,
    clinet = 0x0001,
    server = 0x0002,
    both = clinet | server,
    no_auto_clear = 0x0080,
    no_internal_lookup = 0x0100,
    no_internal_store = 0x0200,
    no_internal = no_internal_lookup | no_internal_store,
    default_flags = off
};

enum class ssl_verify_flags_t : std::uint32_t
{
    none = 0x00,
    peer = 0x01,
    fail_if_no_peer_cert = 0x02,
    peer_id_no_obc = 0x04,
    default_flags = peer | fail_if_no_peer_cert
};

enum class ssl_state_flags_t : std::uint32_t
{
    connect = 0x1000,
    accept = 0x2000,
    mask = 0x0fff,
    init = connect | accept,
    ok = 0x0003,
    renegotiate = 0x0004 | init,
    before = 0x0005 | init,
    loop = 0x0001,
    exit = 0x0002,
    read = 0x0004,
    write = 0x0008,
    alert = 0x4000,
    read_alert = alert | read,
    write_alert = alert | write,
    accept_loop = accept | loop,
    accept_exit = accept | exit,
    connect_loop = connect | loop,
    connect_exit = connect | exit,
    handshake_start = 0x0010,
    handshake_done = 0x0020
};

enum class bio_method_t
{
    default_method,
    in_memory_method,
    fd_method,
    file_method,
    socket_method,
    connect_method
};

enum class ssl_handshake_state_t
{
    ready,
    prepare,
    handshaking,
    done,
    closed,
    failed
};

enum class ssl_state_t
{
    connecting,
    connect,
    accepting,
    accept,
    handshaking,
    shutdown,
    timeout,
    clear,
    renegotiate
};

enum class ssl_control_id_t
{
    handshake,
    shutdown,
    reset,
    retransmit
};

enum class ssl_result_t
{
    ok = 0,
    ssl_error = 1,
    want_read = 2,
    want_write = 3,
    want_x509_lookup = 4,
    error_syscall = 5,
    error_zero_return = 6,
    want_connect = 7,
    want_accept = 8,
    want_channel_id_lookup = 9,
    pending_session = 11,
    pending_cerificate = 12,
    want_private_key_operation = 13,
    pending_ticket = 14,
    early_data_rejected = 15,
    want_certificate_verify = 16,
    handoff = 17,
    handback = 18
};

enum class ssl_shutdown_state_t
{
    none = 0,
    send = 1,
    receive = 2
};

enum class ssl_role_t
{
    undefined = 0,
    server,
    client,
    srvcli
};

enum class ssl_data_type_t
{
    encrypted,
    application
};

enum class fingerprint_direction_t
{
    self,
    peer
};

enum class ssl_alert_type_t
{
    undefined,
    warning,
    fatal
};

}


#endif // SSL_TYPES_H
