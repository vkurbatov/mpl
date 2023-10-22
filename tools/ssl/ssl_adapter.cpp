#include "ssl_adapter.h"
#include "ssl_utils.h"

#include <openssl/ssl.h>

namespace ssl
{

ssl_ptr_t ssl_adapter::create_ssl(const ssl_ctx_ptr_t &ssl_ctx
                                  , const bio_ptr_t &bio_read
                                  , const bio_ptr_t &bio_write)
{
    if (auto ssl = create_object<SSL>(ssl_ctx))
    {
        if (bio_read != nullptr
                && bio_write != nullptr)
        {
            set_bio(ssl
                    , bio_read
                    , bio_write);
        }
        return ssl;
    }

    return nullptr;
}

void ssl_adapter::set_bio(ssl_ptr_t &ssl
                          , const bio_ptr_t &bio_read
                          , const bio_ptr_t &bio_write)
{
    SSL_set_bio(ssl.get()
                , bio_read.get()
                , bio_write.get());
    BIO_up_ref(bio_read.get());
    BIO_up_ref(bio_write.get());
}

void ssl_adapter::set_mtu(ssl_ptr_t &ssl
                          , std::uint32_t mtu)
{
    SSL_set_mtu(ssl.get()
                , mtu);

}

ssl_result_t ssl_adapter::set_state(ssl_ptr_t &ssl, ssl_state_t state)
{
    ssl_result_t result = ssl_result_t::ok;
    switch(state)
    {
        case ssl_state_t::connecting:
            result = get_result(ssl
                                , SSL_connect(ssl.get())
                                );
        break;
        case ssl_state_t::connect:
            SSL_set_connect_state(ssl.get());
        break;
        case ssl_state_t::accepting:
            result = get_result(ssl
                                , SSL_accept(ssl.get())
                                );
        break;
        case ssl_state_t::accept:
            SSL_set_accept_state(ssl.get());
            result = get_result(ssl
                                , SSL_accept(ssl.get())
                                );
        break;
        case ssl_state_t::handshaking:
            result = get_result(ssl
                                , SSL_do_handshake(ssl.get())
                                );
        break;
        case ssl_state_t::shutdown:
            result = get_result(ssl
                                , SSL_shutdown(ssl.get())
                                );
        break;
        case ssl_state_t::timeout: // only dtls
            result = get_result(ssl
                                , DTLSv1_handle_timeout(ssl.get())
                                );
        break;
        case ssl_state_t::clear:
            result = get_result(ssl
                                , SSL_clear(ssl.get())
                                );
        break;
        case ssl_state_t::renegotiate:
            result = get_result(ssl
                                , SSL_renegotiate(ssl.get())
                                );
        break;
    }

    return result;
}

uint64_t ssl_adapter::get_timeout(const ssl_ptr_t &ssl)
{
    uint64_t result = 0;
    struct timeval timeout = { 0, 0 };
    if (DTLSv1_get_timeout(ssl.get()
                           , &timeout))
    {
        result = static_cast<std::uint64_t>(timeout.tv_sec) * 1000000ul
                + timeout.tv_usec;
    }
    return result;
}

bool ssl_adapter::is_server(const ssl_ptr_t &ssl)
{
    return SSL_is_server(ssl.get()) > 0;
}

bool ssl_adapter::is_dtls(const ssl_ptr_t &ssl)
{
    return SSL_is_dtls(ssl.get()) > 0;
}

bool ssl_adapter::is_renegatiation_pending(const ssl_ptr_t &ssl)
{
    return SSL_renegotiate_pending(ssl.get());
}

ssl_result_t ssl_adapter::get_result(const ssl_ptr_t &ssl
                                     , int32_t io_result)
{
    return static_cast<ssl_result_t>(SSL_get_error(ssl.get()
                                                   , io_result));
}

ssl_shutdown_state_t ssl_adapter::get_shutdown_state(const ssl_ptr_t &ssl)
{
    return static_cast<ssl_shutdown_state_t>(SSL_get_shutdown(ssl.get()));
}

std::int32_t ssl_adapter::read(ssl_ptr_t &ssl
                               , void *data
                               , std::size_t size)
{
    return SSL_read(ssl.get()
                    , data
                    , size);
}

std::int32_t ssl_adapter::write(ssl_ptr_t &ssl
                                , const void *data
                                , std::size_t size)
{
    return SSL_write(ssl.get()
                    , data
                    , size);
}

std::size_t ssl_adapter::read(ssl_ptr_t &ssl
                              , void *data
                              , std::size_t size
                              , ssl_result_t &result)
{
    auto io_result = read(ssl
                           , data
                           , size);
    if (io_result < 0)
    {
        result = get_result(ssl
                            , io_result);
        return 0;
    }

    result = ssl_result_t::ok;
    return io_result;

}

std::size_t ssl_adapter::write(ssl_ptr_t &ssl
                               , const void *data
                               , std::size_t size
                               , ssl_result_t &result)
{
    auto io_result = write(ssl
                           , data
                           , size);
    if (io_result < 0)
    {
        result = get_result(ssl
                            , io_result);
        return 0;
    }

    result = ssl_result_t::ok;
    return io_result;
}

x509_ptr_t ssl_adapter::get_peer_certificate(const ssl_ptr_t &ssl)
{
    if (auto x509 = SSL_get_peer_certificate(ssl.get()))
    {
        return create_object<X509>(x509);
    }

    return nullptr;
}

bool ssl_adapter::export_keying_material(ssl_ptr_t &ssl
                                        , const std::string &label
                                        , void *material
                                        , std::size_t material_size)
{
    return SSL_export_keying_material(ssl.get()
                                       , static_cast<std::uint8_t*>(material)
                                       , material_size
                                       , label.c_str()
                                       , label.length()
                                       , nullptr
                                       , 0
                                      , 0) > 0;
}

srtp_profile_id_t ssl_adapter::get_selected_srtp_profile(const ssl_ptr_t &ssl)
{
    if (auto srtp_profile = SSL_get_selected_srtp_profile(ssl.get()))
    {
        return static_cast<srtp_profile_id_t>(srtp_profile->id);
    }

    return srtp_profile_id_t::none;
}


ssl_adapter::ssl_adapter(ssl_ptr_t &&ssl
                         , ssl_state_handler_t state_handler
                         , ssl_verify_handler_t verify_handler)
    : m_ssl(std::move(ssl))
    , m_state_handler(std::move(state_handler))
    , m_verify_handler(std::move(verify_handler))
{
    SSL_set_ex_data(m_ssl.get()
                    , 0
                    , static_cast<void*>(this));

}

ssl_adapter::ssl_adapter(const ssl_ctx_ptr_t &ssl_ctx
                         , const bio_ptr_t &bio_read
                         , const bio_ptr_t &bio_write
                         , ssl_state_handler_t state_handler
                         , ssl_verify_handler_t verify_handler)
    : ssl_adapter(create_ssl(ssl_ctx
                            , bio_read
                            , bio_write)
                  , std::move(state_handler)
                  , std::move(verify_handler)
                  )
{

}

ssl_adapter::~ssl_adapter()
{
    SSL_set_ex_data(m_ssl.get()
                    , 0
                    , nullptr);
}

void ssl_adapter::set_bio(const bio_ptr_t &bio_read, const bio_ptr_t &bio_write)
{
    set_bio(m_ssl, bio_read, bio_write);
}

void ssl_adapter::set_mtu(uint32_t mtu)
{
    set_mtu(m_ssl, mtu);
}

ssl_result_t ssl_adapter::set_state(ssl_state_t state)
{
    return set_state(m_ssl, state);
}

uint64_t ssl_adapter::get_timeout() const
{
    return get_timeout(m_ssl);
}

bool ssl_adapter::is_renegatiation_pending() const
{
    return is_renegatiation_pending(m_ssl);
}

ssl_result_t ssl_adapter::get_result(int32_t io_result) const
{
    return get_result(m_ssl, io_result);
}

ssl_shutdown_state_t ssl_adapter::get_shutdown_state() const
{
    return get_shutdown_state(m_ssl);
}

int32_t ssl_adapter::read(void *data, std::size_t size)
{
    return read(m_ssl, data, size);
}

int32_t ssl_adapter::write(const void *data, std::size_t size)
{
    return write(m_ssl, data, size);
}

std::size_t ssl_adapter::read(void *data, std::size_t size, ssl_result_t &result)
{
    return read(m_ssl, data, size, result);
}

std::size_t ssl_adapter::write(const void *data, std::size_t size, ssl_result_t &result)
{
    return write(m_ssl, data, size, result);
}

x509_ptr_t ssl_adapter::get_peer_certificate() const
{
    return get_peer_certificate(m_ssl);
}

bool ssl_adapter::export_keying_material(const std::string &label, void *material, std::size_t material_size)
{
    return export_keying_material(m_ssl, label, material, material_size);
}

srtp_profile_id_t ssl_adapter::get_selected_srtp_profile() const
{
    return get_selected_srtp_profile(m_ssl);
}

void ssl_adapter::set_state_handler(ssl_state_handler_t state_handler)
{
    m_state_handler = std::move(state_handler);
}

void ssl_adapter::set_verify_handler(ssl_verify_handler_t verify_handler)
{
    m_verify_handler = std::move(verify_handler);
}

void ssl_adapter::set_ssl(ssl_ptr_t &&ssl_ptr)
{
    m_ssl = std::move(ssl_ptr);
    if (m_ssl)
    {
        SSL_set_ex_data(m_ssl.get()
                        , 0
                        , static_cast<void*>(this));
    }
}

void ssl_adapter::set_ssl(const ssl_ctx_ptr_t &ssl_ctx
                          , const bio_ptr_t &bio_read
                          , const bio_ptr_t &bio_write)
{
    set_ssl(create_ssl(ssl_ctx
                       , bio_read
                       , bio_write));
}

const ssl_ptr_t &ssl_adapter::native_handle() const
{
    return m_ssl;
}

ssl_ptr_t ssl_adapter::release()
{
    return std::move(m_ssl);
}

bool ssl_adapter::is_valid() const
{
    return m_ssl != nullptr;
}

void ssl_adapter::on_ssl_info(std::int32_t type, std::int32_t value)
{
    if (m_state_handler != nullptr)
    {
        m_state_handler(type, value);
    }
}

bool ssl_adapter::on_verify(std::int32_t ok
                            , x509_store_ctx_st* ctx)
{
    if (m_verify_handler != nullptr)
    {
        return m_verify_handler(ok
                                , ctx);
    }

    return true;
}

}
