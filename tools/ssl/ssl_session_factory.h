#ifndef SSL_SESSION_FACTORY_H
#define SSL_SESSION_FACTORY_H

#include "i_ssl_session.h"


namespace ssl
{

struct ssl_manager_config_t;

class ssl_session_factory
{
private:
    struct pimpl_t;
    using pimpl_ptr_t = std::unique_ptr<pimpl_t>;

    pimpl_ptr_t     m_pimpl;

public:
    ssl_session_factory(const ssl_manager_config_t& config);
    ~ssl_session_factory();

    const i_ssl_certificate* certificate() const;

    i_ssl_session::u_ptr_t create_session(const ssl_session_params_t& params
                                          , i_ssl_session::i_listener* listener = nullptr);

    bool is_valid() const;
};

}

#endif // SSL_SESSION_FACTORY_H
