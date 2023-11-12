#ifndef SSL_SRTP_SESSION_H
#define SSL_SRTP_SESSION_H

#include "srtp_types.h"
#include <memory>
#include <functional>

namespace pt::ssl
{

struct srtp_packet_t;
struct srtp_session_config_t;

using srtp_packet_handler_t = std::function<void(const srtp_packet_t&)>;

class srtp_session
{
    struct context_t;
    using context_ptr_t = std::shared_ptr<context_t>;

    context_ptr_t       m_context;

public:

    using s_ptr_t = std::shared_ptr<srtp_session>;

    static bool init();
    static bool cleanup();
    static bool is_init();

    static s_ptr_t create(const srtp_session_config_t& config
                            , srtp_packet_handler_t packet_handler = nullptr);

    srtp_session(const srtp_session_config_t& config
                 , srtp_packet_handler_t packet_handler = nullptr);

    const srtp_session_config_t& config() const;

    void set_packet_handler(srtp_packet_handler_t packet_handler);

    void push_packet(const srtp_packet_t& packet);

    bool is_valid() const;
};

}

#endif // SSL_SRTP_SESSION_H
