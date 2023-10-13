#ifndef SSL_EC_KEY_H
#define SSL_EC_KEY_H

#include "ssl_native.h"
#include <cstdint>

namespace ssl
{

class ssl_ec_key
{
    ec_key_ptr_t    m_ec_key;
public:
    static evp_pkey_ptr_t create_evp_pkey(ec_key_ptr_t&& ec_key);
    static ec_key_ptr_t create_ec_key(std::int32_t nid);

    ssl_ec_key(ec_key_ptr_t&& ec_key);
    ssl_ec_key(std::int32_t nid);

    evp_pkey_ptr_t create_evp_pkey();

    const ec_key_ptr_t& native_handle() const;
    ec_key_ptr_t release();
    bool is_valid() const;
};

}

#endif // SSL_EC_KEY_H
