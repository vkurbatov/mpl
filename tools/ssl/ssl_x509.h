#ifndef SSL_X509_H
#define SSL_X509_H

#include "ssl_native.h"
#include "ssl_types.h"
#include <string>
#include <vector>

namespace pt::ssl
{

class ssl_x509
{
    x509_ptr_t  m_x509;

public:

    static bool set_version(x509_ptr_t& x509
                            , std::int32_t version);
    static std::int32_t get_version(const x509_ptr_t& x509);

    static std::size_t digest(const x509_ptr_t& x509
                               , hash_method_t hash_method
                               , std::vector<std::uint8_t>& data);

    static std::size_t digest(const x509_ptr_t& x509
                               , hash_method_t hash_method
                               , void* data);


    static x509_ptr_t create_x509();
    static x509_ptr_t create_x509(const bio_ptr_t& bio);
    static x509_ptr_t create_x509(const std::string& x509_string);
    static x509_ptr_t create_x509(const evp_pkey_ptr_t& evp_pkey
                                  , const std::string& subject_name = {}
                                  , hash_method_t hash_method = hash_method_t::sha1);

    ssl_x509();
    ssl_x509(x509_ptr_t&& x509);
    ssl_x509(const bio_ptr_t& bio);
    ssl_x509(const std::string& x509_string);
    ssl_x509(const evp_pkey_ptr_t& evp_pkey
             , const std::string& subject_name = {}
             , hash_method_t hash_method = hash_method_t::sha1);

    void set(x509_ptr_t&& x509);

    bool set_version(std::int32_t version);
    std::size_t digest(hash_method_t hash_method
                       , void* data) const;

    std::size_t digest(hash_method_t hash_method
                       , std::vector<std::uint8_t>& data) const;

    const x509_ptr_t& native_handle() const;
    x509_ptr_t release();
    bool is_valid() const;
};

}

#endif // SSL_X509_H
