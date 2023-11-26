#ifndef I_SSL_SESSION_H
#define I_SSL_SESSION_H

#include "ssl_types.h"

#include <memory>

namespace pt::ssl
{

struct ssl_session_params_t;
struct srtp_key_info_t;
class i_ssl_message_sink;
class i_ssl_certificate;
class i_ssl_message;

class i_ssl_session
{
public:
    using u_ptr_t = std::unique_ptr<i_ssl_session>;
    using s_ptr_t = std::shared_ptr<i_ssl_session>;

    class i_listener
    {
    public:
        virtual ~i_listener() = default;
        virtual void on_message(const i_ssl_message& message) = 0;
        virtual void on_state(ssl_handshake_state_t state) = 0;
        virtual void on_srtp_key_info(const srtp_key_info_t& encrypted_key
                                      , const srtp_key_info_t& decrypted_key) = 0;
        virtual bool on_verify_certificate(const i_ssl_certificate* remote_certificate) = 0;
        virtual void on_wait_timeout(std::uint64_t timeout) = 0;
        virtual void on_error(ssl_alert_type_t alert_type, const std::string& reason) = 0;
    };

    virtual ~i_ssl_session() = default;

    virtual const ssl_session_params_t& params() const = 0;
    virtual bool set_params(const ssl_session_params_t& params) = 0;
    virtual ssl_handshake_state_t state() const = 0;
    virtual bool control(ssl_session_control_id_t control_id) = 0;
    virtual i_ssl_message_sink* sink() = 0;
    virtual const i_ssl_certificate* local_certificate() const = 0;
    virtual const i_ssl_certificate* remote_certificate() const = 0;
    virtual bool set_listener(i_listener* listener) = 0;

};

}

#endif // I_SSL_SESSION_H
