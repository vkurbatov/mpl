#include "ssl_ec_key.h"
#include "ssl_utils.h"
#include "ssl_private_key.h"
#include <openssl/ssl.h>

namespace ssl
{

evp_pkey_ptr_t ssl_ec_key::create_evp_pkey(ec_key_ptr_t &&ec_key)
{
    if (ec_key != nullptr)
    {
        return ssl_private_key::create_evp_pkey(std::move(ec_key));
    }

    return nullptr;
}

ec_key_ptr_t ssl_ec_key::create_ec_key(int32_t nid)
{
    return create_unique_object<EC_KEY, std::int32_t>(nid);
}

ssl_ec_key::ssl_ec_key(ec_key_ptr_t &&ec_key)
    : m_ec_key(std::move(ec_key))
{

}

ssl_ec_key::ssl_ec_key(int32_t nid)
    : ssl_ec_key(create_ec_key(nid))
{

}

evp_pkey_ptr_t ssl_ec_key::create_evp_pkey()
{
    return create_evp_pkey(std::move(m_ec_key));
}

const ec_key_ptr_t &ssl_ec_key::native_handle() const
{
    return m_ec_key;
}

ec_key_ptr_t ssl_ec_key::release()
{
    return std::move(m_ec_key);
}

bool ssl_ec_key::is_valid() const
{
    return m_ec_key != nullptr;
}

}
