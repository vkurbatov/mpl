#ifndef I_SSL_CONNECTION_OBSERVER_H
#define I_SSL_CONNECTION_OBSERVER_H

#include "ssl_types.h"
#include "i_ssl_message.h"


#include <string>
#include <memory>

namespace ssl
{

struct srtp_key_info_t;

class i_ssl_connection_observer
{
public:
    using s_ptr_t = std::shared_ptr<i_ssl_connection_observer>;
    virtual ~i_ssl_connection_observer() = default;
    virtual void on_message(const i_ssl_message& message) = 0;
    virtual void on_state(ssl_handshake_state_t state) = 0;
    virtual void on_srtp_key_info(const srtp_key_info_t& client_key
                                  , const srtp_key_info_t& server_key) = 0;
    virtual void on_wait_timeout(std::uint64_t timeout) = 0;
    virtual bool on_verify(std::int32_t ok) = 0;
    virtual void on_error(ssl_alert_type_t alert_type, const std::string& reason) = 0;
};

}

#endif // I_SSL_CONNECTION_OBSERVER_H
