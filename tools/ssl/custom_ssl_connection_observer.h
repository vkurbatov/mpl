#ifndef CUSTOM_SSL_CONNECTION_OBSERVER_H
#define CUSTOM_SSL_CONNECTION_OBSERVER_H

#include "i_ssl_connection_observer.h"
#include <functional>

namespace ssl
{

class custom_ssl_connection_observer : public i_ssl_connection_observer
{
public:
    using s_ptr_t = std::shared_ptr<i_ssl_connection_observer>;
    using message_handler_t = std::function<void(const i_ssl_message &)>;
    using state_handler_t = std::function<void(ssl_handshake_state_t)>;
    using srtp_key_handler_t = std::function<void(const srtp_key_info_t&, const srtp_key_info_t&)>;
    using wait_timeout_handler_t = std::function<void(std::uint64_t timeout_us)>;
    using error_handler_t = std::function<void(ssl_alert_type_t, const std::string&)>;
    using verify_handler_t = std::function<bool(std::int32_t ok)>;

private:
    message_handler_t       m_message_handler;
    state_handler_t         m_state_handler;
    srtp_key_handler_t      m_srtp_key_handler;
    error_handler_t         m_error_handler;
    verify_handler_t        m_verify_handler;
    wait_timeout_handler_t  m_wait_timeout_handler;

public:
    static s_ptr_t create(message_handler_t message_handler = nullptr
                            , state_handler_t state_handler = nullptr
                            , srtp_key_handler_t srtp_key_handler = nullptr
                            , error_handler_t error_handler = nullptr
                            , wait_timeout_handler_t wait_timeout_handler = nullptr);

    custom_ssl_connection_observer(message_handler_t message_handler = nullptr
                                   , state_handler_t state_handler = nullptr
                                   , srtp_key_handler_t srtp_key_handler = nullptr
                                   , error_handler_t error_handler = nullptr
                                   , wait_timeout_handler_t wait_timeout_handler = nullptr);

    void set_message_handler(message_handler_t message_handler);
    void set_state_handler(state_handler_t state_handler);
    void set_srtp_key_handler(srtp_key_handler_t srtp_key_handler);
    void set_error_handler(error_handler_t error_handler);
    void set_verify_handler(verify_handler_t verify_handler);
    void set_wait_timeout_handler(wait_timeout_handler_t wait_timeout_handler);

    // i_ssl_connection_observer interface
public:
    void on_message(const i_ssl_message &message) override;
    void on_state(ssl_handshake_state_t state) override;
    void on_srtp_key_info(const srtp_key_info_t &client_key
                          , const srtp_key_info_t &server_key) override;
    void on_wait_timeout(uint64_t timeout) override;
    void on_error(ssl_alert_type_t alert_type, const std::string &reason) override;



    // i_ssl_connection_observer interface
public:
    hash_method_t query_peer_fingerprint(std::vector<uint8_t> &hash) override;
};

}

#endif // CUSTOM_SSL_CONNECTION_OBSERVER_H
