#ifndef SSL_PRIVATE_KEY_H
#define SSL_PRIVATE_KEY_H

#include "ssl_native.h"
#include "ssl_types.h"
#include <string>

namespace pt::ssl
{

class ssl_private_key
{
    evp_pkey_ptr_t  m_evp_pkey;
public:

    static evp_pkey_ptr_t create_evp_pkey(ec_key_ptr_t&& ec_key);
    static evp_pkey_ptr_t create_evp_pkey(const bio_ptr_t& bio);
    static evp_pkey_ptr_t create_evp_pkey(const std::string& private_key_string);

    ssl_private_key(evp_pkey_ptr_t&& evp_pkey);
    ssl_private_key(ec_key_ptr_t&& ec_key);
    ssl_private_key(const bio_ptr_t& bio);
    ssl_private_key(const std::string& private_key_string);

    x509_ptr_t create_x509(const std::string& subject_name
                           , hash_method_t hash_method = hash_method_t::sha1) const;

    const evp_pkey_ptr_t& native_handle() const;
    evp_pkey_ptr_t release();
    bool is_valid() const;
};

}

#endif // SSL_PRIVATE_KEY_H
