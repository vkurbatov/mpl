#include "ssl_x509.h"
#include "ssl_utils.h"
#include "bio_context.h"
#include <openssl/ssl.h>
#include <cstring>

namespace ssl
{


bool ssl_x509::set_version(x509_ptr_t &x509
                           , int32_t version)
{
    return X509_set_version(x509.get()
                            , version) > 0;
}

int32_t ssl_x509::get_version(const x509_ptr_t &x509)
{
    return X509_get_version(x509.get());
}

std::size_t ssl_x509::digest(const x509_ptr_t &x509
                             , hash_method_t hash_method
                             , std::vector<uint8_t> &data)
{
    static thread_local std::vector<uint8_t> storage_buffer(EVP_MAX_MD_SIZE);

    auto size = digest(x509
                       , hash_method
                       , storage_buffer.data());
    if (size > 0)
    {
        data.insert(data.end()
                    , storage_buffer.begin()
                    , storage_buffer.begin() + size);
    }

    return size;
}

std::size_t ssl_x509::digest(const x509_ptr_t &x509
                      , hash_method_t hash_method
                      , void *data)
{
    std::uint32_t len = 0;
    if (X509_digest(x509.get()
                    , get_method<EVP_MD>(hash_method)
                    , reinterpret_cast<std::uint8_t*>(data)
                    , &len) > 0)

    {
        return len;
    }

    return 0;
}

x509_ptr_t ssl_x509::create_x509()
{
    return create_object<X509>();
}

x509_ptr_t ssl_x509::create_x509(const bio_ptr_t &bio)
{
    return create_object<X509, const bio_ptr_t&>(bio);
}

x509_ptr_t ssl_x509::create_x509(const std::string &x509_string)
{
    if (auto bio = bio_context::create_bio(x509_string.c_str()
                                           , x509_string.size()))
    {
        return create_x509(bio);
    }

    return nullptr;
}

x509_ptr_t ssl_x509::create_x509(const evp_pkey_ptr_t& evp_pkey
                                 , const std::string& subject_name
                                 , hash_method_t hash_method)
{
    return create_object<X509
            , const evp_pkey_ptr_t&
            , const std::string&>(evp_pkey
                                  , subject_name
                                  , hash_method);
}

ssl_x509::ssl_x509()
    : ssl_x509(create_x509())
{

}

ssl_x509::ssl_x509(x509_ptr_t &&x509)
    : m_x509(std::move(x509))
{

}

ssl_x509::ssl_x509(const bio_ptr_t &bio)
    : ssl_x509(create_x509(bio))
{

}

ssl_x509::ssl_x509(const std::string &x509_string)
{

}

ssl_x509::ssl_x509(const evp_pkey_ptr_t &evp_pkey
                   , const std::string& subject_name
                   , hash_method_t hash_method)
    : ssl_x509(std::move(create_x509(evp_pkey
                                     , subject_name
                                     , hash_method)))
{

}

bool ssl_x509::set_version(int32_t version)
{
    return set_version(m_x509, version);
}

std::size_t ssl_x509::digest(hash_method_t hash_method
                             , void *data) const
{
    return digest(m_x509
                  , hash_method
                  , data);
}

std::size_t ssl_x509::digest(hash_method_t hash_method
                             , std::vector<uint8_t> &data) const
{
    return digest(m_x509
                  , hash_method
                  , data);
}


const x509_ptr_t &ssl_x509::native_handle() const
{
    return m_x509;
}

x509_ptr_t ssl_x509::release()
{
    return std::move(m_x509);
}

bool ssl_x509::is_valid() const
{
    return m_x509 != nullptr;
}

}
