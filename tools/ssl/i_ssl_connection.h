#ifndef I_SSL_CONNECTION_H
#define I_SSL_CONNECTION_H

#include <memory>
#include "ssl_types.h"
#include "i_ssl_message_sink.h"
#include <vector>
#include <string_view>

namespace ssl
{

using fingerprint_t = std::vector<std::uint8_t>;

struct ssl_connection_config_t;
class i_ssl_connection_observer;

class i_ssl_connection : public i_ssl_message_sink
{
public:
    using s_ptr_t = std::shared_ptr<i_ssl_connection>;
    virtual ~i_ssl_connection() = default;

    virtual const ssl_connection_config_t& config() const = 0;
    virtual bool set_config(const ssl_connection_config_t& ssl_connection_config) = 0;
    virtual ssl_handshake_state_t state() const = 0;
    virtual bool control(ssl_control_id_t control_id) = 0;
    virtual bool set_observer(i_ssl_connection_observer* observer) = 0;
    virtual fingerprint_t get_fingerprint(fingerprint_direction_t direction
                                          , hash_method_t hash_method) const = 0;
};

}

#endif // I_SSL_CONNECTION_H
