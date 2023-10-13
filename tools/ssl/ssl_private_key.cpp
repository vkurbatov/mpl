#include "ssl_private_key.h"
#include "ssl_utils.h"
#include <openssl/ssl.h>
#include "ssl_x509.h"
#include "bio_context.h"

namespace ssl
{

evp_pkey_ptr_t ssl_private_key::create_evp_pkey(ec_key_ptr_t &&ec_key)
{
    return create_object<EVP_PKEY, ec_key_ptr_t &&>(std::move(ec_key));
}

evp_pkey_ptr_t ssl_private_key::create_evp_pkey(const bio_ptr_t &bio)
{
    return create_object<EVP_PKEY, const bio_ptr_t &>(bio);
}

evp_pkey_ptr_t ssl_private_key::create_evp_pkey(const std::string &private_key_string)
{
    if (auto bio = bio_context::create_bio(private_key_string.c_str()
                                           , private_key_string.size()))
    {
        return create_evp_pkey(bio);
    }

    return nullptr;
}

ssl_private_key::ssl_private_key(evp_pkey_ptr_t &&evp_pkey)
    : m_evp_pkey(evp_pkey)
{

}

ssl_private_key::ssl_private_key(ec_key_ptr_t &&ec_key)
    : ssl_private_key(create_evp_pkey(std::move(ec_key)))
{

}

ssl_private_key::ssl_private_key(const bio_ptr_t &bio)
    : ssl_private_key(create_evp_pkey(bio))
{

}

ssl_private_key::ssl_private_key(const std::string &private_key_string)
    : ssl_private_key(create_evp_pkey(private_key_string))
{

}

x509_ptr_t ssl_private_key::create_x509(const std::string &subject_name
                                        , hash_method_t hash_method) const
{
    return ssl_x509::create_x509(m_evp_pkey
                                 , subject_name
                                 , hash_method);
}

const evp_pkey_ptr_t &ssl_private_key::native_handle() const
{
    return m_evp_pkey;
}

evp_pkey_ptr_t ssl_private_key::release()
{
    return std::move(m_evp_pkey);
}

bool ssl_private_key::is_valid() const
{
    return m_evp_pkey != nullptr;
}

}
