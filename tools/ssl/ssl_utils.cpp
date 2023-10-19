#include "ssl_utils.h"
#include <openssl/ssl.h>

#include "ssl_types.h"
#include "srtp_types.h"
#include <functional>
#include <set>
#include <unordered_set>

namespace ssl
{

namespace detail
{

template<typename T>
using ssl_deleter_t = std::function<void(T* pointer)>;

template<typename T, typename F>
ssl_deleter_t<T> create_deleter(F&& f)
{
    return [f](T* p)
    {
        if (p != nullptr)
        {
            (f)(p);
        }
    };
}

template<typename M, typename E, class ...Args>
const M* get_method(E method, Args& ...args)
{
    using function_t = const M*(*)();
    static function_t method_table[] =
    {
        nullptr,
        args...
    };

    if (auto func = method_table[static_cast<std::int32_t>(method)])
    {
        return func();
    }

    return nullptr;
}

template<typename T, typename DeleteFunction>
ssl_s_ptr_t<T> safe_ssl_object(T* native
                                   , DeleteFunction&& delete_function)
{
    if (native != nullptr)
    {
        return ssl_s_ptr_t<T>(native
                                , delete_function);
    }

    return nullptr;
}

template<typename T, typename DeleteFunction>
ssl_unique_s_ptr_t<T> safe_unique_ssl_object(T* native
                                        , DeleteFunction&& delete_function)
{
    if (native != nullptr)
    {
        return ssl_unique_s_ptr_t<T>(native
                                     , delete_function);
    }

    return ssl_unique_s_ptr_t<T>(nullptr
                                   , nullptr);
}


template<typename T, typename CreateFunction, typename DeleteFunction, class ...Args>
ssl_s_ptr_t<T> create_ssl_object(CreateFunction&& create_function
                                   , DeleteFunction&& delete_function
                                   , Args&& ...args)
{
    if (auto native = create_function(args...))
    {
        return safe_ssl_object(native
                               , std::move(delete_function));
    }

    return nullptr;
}

template<typename T, typename CreateFunction, typename DeleteFunction, class ...Args>
ssl_unique_s_ptr_t<T> create_unique_ssl_object(CreateFunction&& create_function
                                               , DeleteFunction&& delete_function
                                               , Args&& ...args)
{
    if (auto native = create_function(args...))
    {
        return safe_unique_ssl_object(native
                                      , std::move(delete_function));
    }

    return ssl_unique_s_ptr_t<T>(nullptr
                                   , nullptr);
}

const SSL_METHOD* get_ssl_method(ssl_method_t method)
{
    return get_method<SSL_METHOD, ssl_method_t>(method
                                                , TLS_method
                                                , DTLS_method);
}

const BIO_METHOD* get_bio_method(bio_method_t method)
{
    return get_method<BIO_METHOD, bio_method_t>(method
                                                , BIO_s_mem
                                                , BIO_s_fd
                                                , BIO_s_file
                                                , BIO_s_socket
                                                , BIO_s_connect);
}

const EVP_MD* get_hash_method(hash_method_t method)
{
    return get_method<EVP_MD, std::int32_t>(static_cast<std::int32_t>(method)
                                             , EVP_md4
                                             , EVP_md5
                                             , EVP_sha1
                                             , EVP_sha224
                                             , EVP_sha256
                                             , EVP_sha384
                                             , EVP_sha512
                                             , EVP_md5_sha1);
}

}


template<>
const SSL_METHOD *get_method(ssl_method_t method)
{
    return detail::get_ssl_method(method);
}

template<>
const BIO_METHOD *get_method(bio_method_t method)
{
    return detail::get_bio_method(method);
}


template<>
const EVP_MD *get_method(hash_method_t method)
{
    return detail::get_hash_method(method);
}

template<>
ssl_s_ptr_t<SSL_CTX> create_object(SSL_CTX* ssl_ctx)
{
    return detail::safe_ssl_object<SSL_CTX>(ssl_ctx
                                            , SSL_CTX_free);
}

template<>
ssl_s_ptr_t<SSL_CTX> create_object(ssl_method_t method)
{
    return detail::create_ssl_object<SSL_CTX>(SSL_CTX_new
                                                , SSL_CTX_free
                                                , get_method<SSL_METHOD>(method));
}


template<>
ssl_s_ptr_t<SSL> create_object(ssl_s_ptr_t<SSL_CTX> ssl_context)
{
    return detail::create_ssl_object<SSL>(SSL_new
                                          , SSL_free
                                          , ssl_context.get());
}

template<>
ssl_s_ptr_t<BIO> create_object(bio_method_t method)
{
    return detail::create_ssl_object<BIO>(BIO_new
                                          , BIO_free
                                          , get_method<BIO_METHOD>(method));
}

template<>
ssl_s_ptr_t<BIO> create_object(const BIO_METHOD* method)
{
    return detail::create_ssl_object<BIO>(BIO_new
                                          , BIO_free
                                          , method);
}

template<>
ssl_s_ptr_t<BIO> create_object(const std::string& file_name
                                 , const std::string& mode)
{
    return detail::create_ssl_object<BIO>(BIO_new_file
                                          , BIO_free
                                          , file_name.c_str()
                                          , mode.c_str());
}

template<>
ssl_s_ptr_t<BIO> create_object(const void* data
                                 , std::size_t size)
{
    return detail::create_ssl_object<BIO>(BIO_new_mem_buf
                                          , BIO_free
                                          , data
                                          , size);
}

template<>
ssl_s_ptr_t<X509> create_object(X509 *x509)
{
    return detail::safe_ssl_object<X509>(x509
                                         , X509_free);
}

template<>
ssl_s_ptr_t<X509> create_object()
{
    return detail::create_ssl_object<X509>(X509_new
                                           , X509_free);
}

template<>
ssl_s_ptr_t<X509> create_object(const ssl_s_ptr_t<EVP_PKEY>& evp_pkey
                                  , const std::string& subject
                                  , hash_method_t hash_method)
{
    constexpr long lo_cert_time = 0L;
    constexpr long hi_cert_time = 31536000L;
    if (auto x509 = create_object<X509>())
    {
        X509_set_version(x509.get(), 2);

        ASN1_INTEGER_set(X509_get_serialNumber(x509.get())
                         , 1);
        X509_gmtime_adj(X509_get_notBefore(x509.get())
                        , lo_cert_time);
        X509_gmtime_adj(X509_get_notAfter(x509.get())
                        , hi_cert_time);

        if (X509_set_pubkey(x509.get()
                            , evp_pkey.get()) > 0)
        {
            if (auto name = X509_get_subject_name(x509.get()))
            {
                if (!subject.empty())
                {
                    X509_NAME_add_entry_by_txt(name
                                               , "C"
                                               ,  MBSTRING_ASC
                                               , reinterpret_cast<const unsigned char*>(subject.c_str())
                                               , -1
                                               , -1
                                               , 0);
                    X509_NAME_add_entry_by_txt(name
                                               , "CN"
                                               , MBSTRING_ASC
                                               , reinterpret_cast<const unsigned char*>(subject.c_str())
                                               , -1
                                               , -1
                                               , 0);
                }

                if (X509_set_issuer_name(x509.get()
                                         , name) > 0)
                {
                    if (X509_sign(x509.get()
                                  , evp_pkey.get()
                                  , get_method<EVP_MD>(hash_method)) > 0)
                    {
                        return x509;
                    }
                }
            }
        }
    }

    return nullptr;
}

template<>
ssl_s_ptr_t<X509> create_object(const ssl_s_ptr_t<BIO>& bio)
{
    if (bio)
    {
        if (auto x509 = PEM_read_bio_X509(bio.get(), nullptr, nullptr, nullptr))
        {
            return create_object<X509>(x509);
        }
    }

    return nullptr;
}

template<>
ssl_s_ptr_t<EVP_PKEY> create_object()
{
    return detail::create_ssl_object<EVP_PKEY>(EVP_PKEY_new
                                               , EVP_PKEY_free);
}

template<>
ssl_s_ptr_t<EVP_PKEY> create_object(EVP_PKEY *evp_pkey)
{
    return detail::safe_ssl_object<EVP_PKEY>(evp_pkey
                                         , EVP_PKEY_free);
}

template<>
ssl_s_ptr_t<EVP_PKEY> create_object(ssl_unique_s_ptr_t<EC_KEY>&& ec_key)
{
    if (auto evp_pkey = create_object<EVP_PKEY>())
    {
        if (EVP_PKEY_assign_EC_KEY(evp_pkey.get()
                                   , ec_key.get()) > 0)
        {
            ec_key.release();
            return evp_pkey;
        }
    }

    return nullptr;
}

template<>
ssl_s_ptr_t<EVP_PKEY> create_object(const ssl_s_ptr_t<BIO>& bio)
{
    if (bio)
    {
        if (auto evp_pkey = PEM_read_bio_PrivateKey(bio.get(), nullptr, nullptr, nullptr))
        {
            return create_object<EVP_PKEY>(evp_pkey);
        }
    }

    return nullptr;
}

template<>
ssl_unique_s_ptr_t<EC_KEY> create_unique_object()
{
    return detail::create_unique_ssl_object<EC_KEY>(EC_KEY_new
                                                    , EC_KEY_free);
}


template<>
ssl_unique_s_ptr_t<EC_KEY> create_unique_object(std::int32_t nid)
{
    if (auto ec_key = detail::create_unique_ssl_object<EC_KEY>(EC_KEY_new_by_curve_name
                                                               , EC_KEY_free
                                                               , nid))
    {
        EC_KEY_set_asn1_flag(ec_key.get(), OPENSSL_EC_NAMED_CURVE);
        if (EC_KEY_generate_key(ec_key.get()) > 0)
        {
            return ec_key;
        }
    }

    return ssl_unique_s_ptr_t<EC_KEY>(nullptr
                                        , nullptr);
}


#define declare_enum_flag_function(type)\
    template ssl::type& ssl::set_enum_flag(ssl::type&, ssl::type, bool);\
    template bool ssl::get_enum_flag(const ssl::type&, ssl::type);


declare_enum_flag_function(ssl_options_flags_t)
declare_enum_flag_function(ssl_session_cache_flags_t)
declare_enum_flag_function(ssl_verify_flags_t)
declare_enum_flag_function(ssl_state_flags_t)

template<typename T>
T &set_enum_flag(T &flags, T flag, bool enable)
{
    if (enable)
    {
        reinterpret_cast<std::uint32_t&>(flags) |= static_cast<std::uint32_t>(flag);
    }
    else
    {
        reinterpret_cast<std::uint32_t&>(flags) &= ~static_cast<std::uint32_t>(flag);
    }

    return flags;
}

template<typename T>
bool get_enum_flag(const T &flags, T flag)
{
    return (reinterpret_cast<const std::uint32_t&>(flags) & static_cast<std::uint32_t>(flag)) == static_cast<std::uint32_t>(flag);
} 

static const std::string srtp_profile_name_table[] =
{
    "",
    "SRTP_AES128_CM_SHA1_80",
    "SRTP_AES128_CM_SHA1_32",
    "SRTP_AES128_F8_SHA1_80",
    "SRTP_AES128_F8_SHA1_32",
    "SRTP_NULL_SHA1_80",
    "SRTP_NULL_SHA1_32",
    "SRTP_AEAD_AES_128_GCM",
    "SRTP_AEAD_AES_256_GCM"
};

template<>
std::string to_string(const srtp_profile_id_t& profile_id)
{
    return srtp_profile_name_table[static_cast<std::int32_t>(profile_id)];
}

template<>
bool from_string(const std::string &string_value, srtp_profile_id_t &profile_id)
{
    std::int32_t idx = 0;
    for (const auto& s : srtp_profile_name_table)
    {
        if (s == string_value)
        {
            profile_id = static_cast<srtp_profile_id_t>(idx);
            return true;
        }
        idx++;
    }
    return false;
}

template<>
std::string to_string(const srtp_profile_id_set_t& srtp_profile_set)
{
    std::string params;
    for (const auto& s : srtp_profile_set)
    {
        auto srtp_profile_name = to_string<srtp_profile_id_t>(s);
        if (!srtp_profile_name.empty())
        {
            if (!params.empty())
            {
                params.append(":");
            }
            params.append(srtp_profile_name);
        }
    }

    return params;
}

template<>
ssl_version_t convert(std::uint16_t octets)
{
    static const std::unordered_set<ssl_version_t> mapped_table =
    {
        ssl_version_t::ssl3,
        ssl_version_t::tls1,
        ssl_version_t::tls1_1,
        ssl_version_t::tls1_2,
        ssl_version_t::tls1_3,
        ssl_version_t::dtls1,
        ssl_version_t::dtls1_2
    };

    auto it = mapped_table.find(static_cast<ssl_version_t>(octets));
    if (it != mapped_table.end())
    {
        return *it;
    }

    return ssl_version_t::undefined;
}

template<>
ssl_content_type_t convert(std::uint8_t octets)
{
    auto contet_type = static_cast<ssl_content_type_t>(octets);
    if (contet_type >= ssl_content_type_t::change_cipher_spec
            && contet_type <= ssl_content_type_t::application_data)
    {
        return contet_type;
    }

    return ssl_content_type_t::undefined;
}

}

