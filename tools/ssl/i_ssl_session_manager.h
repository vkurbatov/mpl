#ifndef I_SSL_SESSION_MANAGER_H
#define I_SSL_SESSION_MANAGER_H

#include "i_ssl_session.h"

namespace ssl
{

class i_ssl_session_manager
{
public:
    using u_ptr_t = std::unique_ptr<i_ssl_session_manager>;
    using s_ptr_t = std::shared_ptr<i_ssl_session_manager>;

    virtual const i_ssl_certificate* certificate() const = 0;

    virtual i_ssl_session::u_ptr_t create_session(const ssl_session_params_t& params
                                          , i_ssl_session::i_listener* listener = nullptr) = 0;
};

}

#endif // I_SSL_SESSION_MANAGER_H
