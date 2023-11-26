#include "ssl_context.h"
#include "ssl_context_config.h"
#include "ssl_adapter.h"

#include <openssl/ssl.h>


#include <functional>

namespace pt::ssl
{

bool ssl_context::use_x509(ssl_ctx_ptr_t &ssl_ctx, const x509_ptr_t &x509)
{
    return SSL_CTX_use_certificate(ssl_ctx.get()
                                   , x509.get()) > 0;
}

bool ssl_context::use_evp_pkey(ssl_ctx_ptr_t &ssl_ctx
                               , const evp_pkey_ptr_t &evp_pkey)
{
    return SSL_CTX_use_PrivateKey(ssl_ctx.get()
                                  , evp_pkey.get()) > 0;
}

bool ssl_context::check_private_key(const ssl_ctx_ptr_t &ssl_ctx)
{
    return SSL_CTX_check_private_key(ssl_ctx.get()) > 0;
}

void ssl_context::set_read_ahead(ssl_ctx_ptr_t &ssl_ctx, bool enable)
{
    SSL_CTX_set_read_ahead(ssl_ctx.get(), enable ? 1 : 0);
}

void ssl_context::set_verify_depth(ssl_ctx_ptr_t &ssl_ctx
                                   , int32_t depth)
{
    SSL_CTX_set_verify_depth(ssl_ctx.get(), depth);
}

bool ssl_context::set_cipher_list(ssl_ctx_ptr_t &ssl_ctx, const std::string &list)
{
    return SSL_CTX_set_cipher_list(ssl_ctx.get(), list.c_str()) > 0;
}

void ssl_context::set_ecdh_auto(ssl_ctx_ptr_t &ssl_ctx, bool enable)
{
    SSL_CTX_set_ecdh_auto(ssl_ctx.get(), enable ? 1 : 0);
}

bool ssl_context::set_tlsext_use_srtp(ssl_ctx_ptr_t &ssl_ctx, const std::string &list)
{
    return SSL_CTX_set_tlsext_use_srtp(ssl_ctx.get(), list.c_str()) > 0;
}

bool ssl_context::set_options(ssl_ctx_ptr_t &ssl_ctx, ssl_options_flags_t options)
{
    return SSL_CTX_set_options(ssl_ctx.get()
                               , static_cast<std::uint32_t>(options)) > 0;
}

bool ssl_context::set_session_cache_flags(ssl_ctx_ptr_t &ssl_ctx, ssl_session_cache_flags_t session_flags)
{
    return SSL_CTX_set_session_cache_mode(ssl_ctx.get()
                                          , static_cast<std::uint32_t>(session_flags)) > 0;
}

ssl_options_flags_t ssl_context::get_options(const ssl_ctx_ptr_t &ssl_ctx)
{
    return static_cast<ssl_options_flags_t>(SSL_CTX_get_options(ssl_ctx.get()));
}

ssl_session_cache_flags_t ssl_context::get_session_cache_flags(const ssl_ctx_ptr_t &ssl_ctx)
{
    return static_cast<ssl_session_cache_flags_t>(SSL_CTX_get_session_cache_mode(ssl_ctx.get()));
}

void ssl_context::set_verify(ssl_ctx_ptr_t& ssl_ctx, ssl_verify_flags_t verify_flags)
{
    static auto verify_handler = [](std::int32_t ok
                                    , X509_STORE_CTX* x509_store_ctx)
    {
        if (auto ssl = static_cast<SSL*>(X509_STORE_CTX_get_ex_data(x509_store_ctx
                                                                         , SSL_get_ex_data_X509_STORE_CTX_idx()
                                                                         )
                                              ))
        {
            // auto cert = X509_STORE_CTX_get_current_cert(x509_store_ctx);
            // auto peer2 = SSL_get_peer_certificate(ssl);
            if (auto adapter = static_cast<ssl_adapter*>(SSL_get_ex_data(ssl
                                                                         , 0)
                                                         ))
            {
                return static_cast<std::int32_t>(adapter->on_verify(ok
                                                                    , x509_store_ctx));
            }
        }

        return 0;
    };
    return SSL_CTX_set_verify(ssl_ctx.get()
                              , static_cast<std::uint32_t>(verify_flags)
                              , verify_handler);
}

ssl_verify_flags_t ssl_context::get_verify(const ssl_ctx_ptr_t &ssl_ctx)
{
    return static_cast<ssl_verify_flags_t>(SSL_CTX_get_verify_mode(ssl_ctx.get()));
}

ssl_ctx_ptr_t ssl_context::create_ssl_ctx(ssl_method_t method)
{
    return create_object<SSL_CTX>(method);
}

ssl_ctx_ptr_t ssl_context::create_ssl_ctx(const ssl_context_config_t &config
                                          , const x509_ptr_t &x509
                                          , const evp_pkey_ptr_t &evp_pkey)
{
    if (auto ssl_ctx = create_ssl_ctx(config.method))
    {
        if (set_options(ssl_ctx, config.options)
                && set_session_cache_flags(ssl_ctx, config.cache_flags))
        {
            set_verify(ssl_ctx, config.verify_flags);
            if (x509 == nullptr
                    || use_x509(ssl_ctx
                                , x509))
            {
                if (evp_pkey == nullptr
                        || use_evp_pkey(ssl_ctx
                                        , evp_pkey) && check_private_key(ssl_ctx))
                {
                    return ssl_ctx;
                }
            }
        }
    }
    return nullptr;
}

ssl_context::ssl_context(ssl_ctx_ptr_t &&ssl_ctx)
    : m_ssl_ctx(std::move(ssl_ctx))
{
    SSL_CTX_set_ex_data(m_ssl_ctx.get()
                        , 0
                        , static_cast<void*>(this));

    static auto on_ssl_info = [](const SSL* ssl, int type, int value)
    {
        if (auto ssl_object = static_cast<ssl_adapter*>(SSL_get_ex_data(ssl, 0)))
        {
            ssl_object->on_ssl_info(type, value);
        }
    };

    SSL_CTX_set_info_callback(m_ssl_ctx.get(), on_ssl_info);
}

ssl_context::ssl_context(const ssl_context_config_t &config
                         , const x509_ptr_t& x509
                         , const evp_pkey_ptr_t& evp_pkey)
    : ssl_context(create_ssl_ctx(config
                                 , x509
                                 , evp_pkey))
{

}

ssl_context::~ssl_context()
{

    SSL_CTX_set_info_callback(m_ssl_ctx.get(), nullptr);

    SSL_CTX_set_ex_data(m_ssl_ctx.get()
                        , 0
                        , nullptr);
}

bool ssl_context::use_x509(const x509_ptr_t &x509)
{
    return use_x509(m_ssl_ctx
                    , x509);
}

bool ssl_context::use_evp_pkey(const evp_pkey_ptr_t &evp_pkey)
{
    return use_evp_pkey(m_ssl_ctx
                        , evp_pkey);
}

bool ssl_context::check_private_key() const
{
    return check_private_key(m_ssl_ctx);
}

void ssl_context::set_read_ahead(bool enable)
{
    set_read_ahead(m_ssl_ctx, enable);
}

void ssl_context::set_verify_depth(int32_t depth)
{
    set_verify_depth(m_ssl_ctx, depth);
}

bool ssl_context::set_cipher_list(const std::string &list)
{
    return set_cipher_list(m_ssl_ctx, list);
}

void ssl_context::set_ecdh_auto(bool enable)
{
    set_ecdh_auto(m_ssl_ctx, enable);
}

bool ssl_context::set_tlsext_use_srtp(const std::string &list)
{
    return set_tlsext_use_srtp(m_ssl_ctx, list);
}

bool ssl_context::set_options(ssl_options_flags_t options)
{
    return set_options(m_ssl_ctx
                       , options);
}

ssl_options_flags_t ssl_context::get_options() const
{
    return get_options(m_ssl_ctx);
}

bool ssl_context::set_session_cache_flags(ssl_session_cache_flags_t session_flags)
{
    return set_session_cache_flags(m_ssl_ctx
                                   , session_flags);
}

ssl_session_cache_flags_t ssl_context::get_session_cache_flags() const
{
    return get_session_cache_flags(m_ssl_ctx);
}

void ssl_context::set_verify(ssl_verify_flags_t verify_flags)
{
    set_verify(m_ssl_ctx
               , verify_flags);
}

ssl_verify_flags_t ssl_context::get_verify() const
{
    return get_verify(m_ssl_ctx);
}

ssl_ptr_t ssl_context::create_ssl() const
{
    return ssl_adapter::create_ssl(m_ssl_ctx);
}

const ssl_ctx_ptr_t &ssl_context::native_handle() const
{
    return m_ssl_ctx;
}

ssl_ctx_ptr_t ssl_context::release()
{
    return std::move(m_ssl_ctx);
}

bool ssl_context::is_valid() const
{
    return m_ssl_ctx != nullptr;
}

std::int32_t ssl_context::on_verify(std::int32_t verify_ok)
{
    return verify_ok;
}


}
