#ifndef I_SSL_MESSAGE_SINK_H
#define I_SSL_MESSAGE_SINK_H

#include <memory>

namespace pt::ssl
{

enum class ssl_io_result_t
{
    ok = 0,
    async,
    bysy,
    not_impl,
    failed,
};

class i_ssl_message;

class i_ssl_message_sink
{
public:
    virtual ~i_ssl_message_sink() = default;
    virtual ssl_io_result_t send_message(const i_ssl_message& message) = 0;
};

}

#endif // I_SSL_MESSAGE_SINK_H
