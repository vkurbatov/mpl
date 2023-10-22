#ifndef SSL_CONTEXT_H
#define SSL_CONTEXT_H

#include "ssl_types.h"
#include "ssl_utils.h"
#include "ssl_native.h"

#include <functional>

namespace pt::ssl
{

struct ssl_context_config_t;

class ssl_context
{
    ssl_ctx_ptr_t   m_ssl_ctx;
public:

    static bool use_x509(ssl_ctx_ptr_t& ssl_ctx, const x509_ptr_t& x509);
    static bool use_evp_pkey(ssl_ctx_ptr_t& ssl_ctx, const evp_pkey_ptr_t& evp_pkey);
    static bool check_private_key(const ssl_ctx_ptr_t& ssl_ctx);
    static void set_read_ahead(ssl_ctx_ptr_t& ssl_ctx, bool enable);
    static void set_verify_depth(ssl_ctx_ptr_t& ssl_ctx, std::int32_t depth);
    static bool set_cipher_list(ssl_ctx_ptr_t& ssl_ctx, const std::string& list);
    static void set_ecdh_auto(ssl_ctx_ptr_t& ssl_ctx, bool enable);
    static bool set_tlsext_use_srtp(ssl_ctx_ptr_t& ssl_ctx, const std::string& list);

    static bool set_options(ssl_ctx_ptr_t& ssl_ctx, ssl_options_flags_t options);
    static ssl_options_flags_t get_options(const ssl_ctx_ptr_t& ssl_ctx);

    static bool set_session_cache_flags(ssl_ctx_ptr_t& ssl_ctx, ssl_session_cache_flags_t session_flags);
    static ssl_session_cache_flags_t get_session_cache_flags(const ssl_ctx_ptr_t& ssl_ctx);

    static void set_verify(ssl_ctx_ptr_t& ssl_ctx, ssl_verify_flags_t verify_flags);
    static ssl_verify_flags_t get_verify(const ssl_ctx_ptr_t& ssl_ctx);

    static ssl_ctx_ptr_t create_ssl_ctx(ssl_method_t method);
    static ssl_ctx_ptr_t create_ssl_ctx(const ssl_context_config_t& config
                                        , const x509_ptr_t& x509 = nullptr
                                        , const evp_pkey_ptr_t& evp_pkey = nullptr);

    ssl_context(ssl_ctx_ptr_t&& ssl_ctx);
    ssl_context(const ssl_context_config_t& config
                , const x509_ptr_t& x509 = nullptr
                , const evp_pkey_ptr_t& evp_pkey = nullptr);

    ~ssl_context();

    bool use_x509(const x509_ptr_t& x509);
    bool use_evp_pkey(const evp_pkey_ptr_t& evp_pkey);
    bool check_private_key() const;

    void set_read_ahead(bool enable);
    void set_verify_depth(std::int32_t depth);
    bool set_cipher_list(const std::string& list);
    void set_ecdh_auto(bool enable);
    bool set_tlsext_use_srtp(const std::string& list);

    bool set_options(ssl_options_flags_t options);
    ssl_options_flags_t get_options() const;

    bool set_session_cache_flags(ssl_session_cache_flags_t session_flags);
    ssl_session_cache_flags_t get_session_cache_flags() const;

    void set_verify(ssl_verify_flags_t verify_flags);
    ssl_verify_flags_t get_verify() const;

    ssl_ptr_t create_ssl() const;

    const ssl_ctx_ptr_t& native_handle() const;
    ssl_ctx_ptr_t release();
    bool is_valid() const;

private:
    std::int32_t on_verify(std::int32_t verify_ok);

};

}

#endif // SSL_CONTEXT_H
