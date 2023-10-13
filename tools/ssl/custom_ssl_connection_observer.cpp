#include "custom_ssl_connection_observer.h"

namespace ssl
{

custom_ssl_connection_observer::s_ptr_t custom_ssl_connection_observer::create(message_handler_t message_handler
                                                                                 , state_handler_t state_handler
                                                                                 , srtp_key_handler_t srtp_key_handler
                                                                                 , error_handler_t error_handler
                                                                                 , verify_handler_t verify_handler
                                                                                 , wait_timeout_handler_t wait_timeout_handler)
{
    return std::make_shared<custom_ssl_connection_observer>(std::move(message_handler)
                                                            , std::move(state_handler)
                                                            , std::move(srtp_key_handler)
                                                            , std::move(error_handler)
                                                            , std::move(verify_handler)
                                                            , std::move(wait_timeout_handler));
}

custom_ssl_connection_observer::custom_ssl_connection_observer(message_handler_t message_handler
                                                               , state_handler_t state_handler
                                                               , srtp_key_handler_t srtp_key_handler
                                                               , error_handler_t error_handler
                                                               , verify_handler_t verify_handler
                                                               , wait_timeout_handler_t wait_timeout_handler)
    : m_message_handler(std::move(message_handler))
    , m_state_handler(std::move(state_handler))
    , m_srtp_key_handler(std::move(srtp_key_handler))
    , m_error_handler(std::move(error_handler))
    , m_verify_handler(std::move(verify_handler))
    , m_wait_timeout_handler(std::move(wait_timeout_handler))
{

}

void custom_ssl_connection_observer::set_message_handler(message_handler_t message_handler)
{
    m_message_handler = std::move(message_handler);
}

void custom_ssl_connection_observer::set_state_handler(state_handler_t state_handler)
{
    m_state_handler = std::move(state_handler);
}

void custom_ssl_connection_observer::set_srtp_key_handler(srtp_key_handler_t srtp_key_handler)
{
    m_srtp_key_handler = std::move(srtp_key_handler);
}

void custom_ssl_connection_observer::set_error_handler(error_handler_t error_handler)
{
    m_error_handler = std::move(error_handler);
}

void custom_ssl_connection_observer::set_verify_handler(verify_handler_t verify_handler)
{
    m_verify_handler = std::move(verify_handler);
}

void custom_ssl_connection_observer::set_wait_timeout_handler(custom_ssl_connection_observer::wait_timeout_handler_t wait_timeout_handler)
{
    m_wait_timeout_handler = std::move(wait_timeout_handler);
}

void custom_ssl_connection_observer::on_message(const i_ssl_message &message)
{
    if (m_message_handler != nullptr)
    {
        return m_message_handler(message);
    }
}

void custom_ssl_connection_observer::on_state(ssl_handshake_state_t state)
{
    if (m_state_handler != nullptr)
    {
        return m_state_handler(state);
    }
}

void custom_ssl_connection_observer::on_srtp_key_info(const srtp_key_info_t &client_key
                                                      , const srtp_key_info_t &server_key)
{
    if (m_srtp_key_handler != nullptr)
    {
        return m_srtp_key_handler(client_key
                                  , server_key);
    }
}

void custom_ssl_connection_observer::on_wait_timeout(uint64_t timeout)
{
    if (m_wait_timeout_handler != nullptr)
    {
        return m_wait_timeout_handler(timeout);
    }
}

bool custom_ssl_connection_observer::on_verify(int32_t ok)
{
    if (m_verify_handler != nullptr)
    {
        return m_verify_handler(ok);
    }

    return true;
}

void custom_ssl_connection_observer::on_error(ssl_alert_type_t alert_type
                                              , const std::string &reason)
{
    if (m_error_handler != nullptr)
    {
        return m_error_handler(alert_type
                               , reason);
    }
}


}
