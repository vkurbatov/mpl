#ifndef SSL_MANAGER_H
#define SSL_MANAGER_H

#include "ssl_types.h"
#include "i_ssl_connection.h"
#include "i_ssl_connection_observer.h"
#include <memory>

namespace ssl
{

struct ssl_manager_config_t;
struct ssl_connection_config_t;
class i_ssl_connection_observer;

class ssl_manager
{
    struct context_t;
    using context_ptr_t = std::shared_ptr<context_t>;

    context_ptr_t           m_context;

public:
    ssl_manager(const ssl_manager_config_t& manager_config);

    std::size_t get_fingerprint(fingerprint_t &fingerprint
                                , hash_method_t hash_method) const;

    i_ssl_connection::s_ptr_t create_connection(const ssl_connection_config_t& connection_config
                                                  , i_ssl_connection_observer* observer);


    bool is_valid() const;
};

}

#endif // SSL_MANAGER_H
