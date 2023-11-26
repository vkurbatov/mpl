#ifndef SSL_ADAPTER_H
#define SSL_ADAPTER_H

#include "ssl_native.h"
#include "ssl_types.h"
#include "srtp_types.h"
#include <functional>

namespace pt::ssl
{

class ssl_context;
using ssl_state_handler_t = std::function<void(int, int)>;
using ssl_verify_handler_t = std::function<bool(std::int32_t ok, x509_store_ctx_st* cert)>;


class ssl_adapter
{
    ssl_ptr_t               m_ssl;
    ssl_state_handler_t     m_state_handler;
    ssl_verify_handler_t    m_verify_handler;

    friend class ssl_context;

public:
    static ssl_ptr_t create_ssl(const ssl_ctx_ptr_t& ssl_ctx
                                , const bio_ptr_t& bio_read = nullptr
                                , const bio_ptr_t& bio_write = nullptr);

    static void set_bio(const ssl_ptr_t& ssl
                        , const bio_ptr_t& bio_read
                        , const bio_ptr_t& bio_write);

    static void set_mtu(const ssl_ptr_t& ssl, std::uint32_t mtu);
    static ssl_result_t control(const ssl_ptr_t& ssl, ssl_control_id_t control);
    static std::uint64_t get_timeout(const ssl_ptr_t& ssl);

    static bool is_server(const ssl_ptr_t& ssl);
    static bool is_dtls(const ssl_ptr_t& ssl);
    static bool is_renegatiation_pending(const ssl_ptr_t& ssl);

    static ssl_result_t get_result(const ssl_ptr_t& ssl, std::int32_t io_result);
    static ssl_shutdown_state_t get_shutdown_state(const ssl_ptr_t& ssl);

    static std::int32_t read(const ssl_ptr_t& ssl, void* data, std::size_t size);
    static std::int32_t write(const ssl_ptr_t& ssl, const void* data, std::size_t size);

    static std::size_t read(const ssl_ptr_t& ssl, void* data, std::size_t size, ssl_result_t& result);
    static std::size_t write(const ssl_ptr_t& ssl, const void* data, std::size_t size, ssl_result_t& result);

    static x509_ptr_t get_peer_certificate(const ssl_ptr_t& ssl);
    static bool export_keying_material(const ssl_ptr_t& ssl
                                       , const std::string& label
                                       , void* material
                                       , std::size_t material_size);
    static srtp_profile_id_t get_selected_srtp_profile(const ssl_ptr_t& ssl);

    ssl_adapter(ssl_ptr_t&& ssl
                , ssl_state_handler_t state_handler = nullptr
                , ssl_verify_handler_t verify_handler = nullptr);
    ssl_adapter(const ssl_ctx_ptr_t& ssl_ctx
                , const bio_ptr_t& bio_read = nullptr
                , const bio_ptr_t& bio_write = nullptr
                , ssl_state_handler_t state_handler = nullptr
                , ssl_verify_handler_t verify_handler = nullptr);

    ~ssl_adapter();

    void set_bio(const bio_ptr_t& bio_read
                 , const bio_ptr_t& bio_write);

    void set_mtu(std::uint32_t mtu);
    ssl_result_t control(ssl_control_id_t control_id);
    std::uint64_t get_timeout() const;

    bool is_server() const;
    bool is_dtls() const;
    bool is_renegatiation_pending() const;

    ssl_result_t get_result(std::int32_t io_result) const;
    ssl_shutdown_state_t get_shutdown_state() const;

    std::int32_t read(void* data, std::size_t size);
    std::int32_t write(const void* data, std::size_t size);

    std::size_t read(void* data, std::size_t size, ssl_result_t& result);
    std::size_t write(const void* data, std::size_t size, ssl_result_t& result);

    x509_ptr_t get_peer_certificate() const;
    bool export_keying_material(const std::string& label, void* material, std::size_t material_size);
    srtp_profile_id_t get_selected_srtp_profile() const;

    void set_state_handler(ssl_state_handler_t state_handler);
    void set_verify_handler(ssl_verify_handler_t verify_handler);

    void set_ssl(ssl_ptr_t&& ssl_ptr);
    void set_ssl(const ssl_ctx_ptr_t& ssl_ctx
                 , const bio_ptr_t& bio_read
                 , const bio_ptr_t& bio_write);

    const ssl_ptr_t& native_handle() const;
    ssl_ptr_t release();
    bool is_valid() const;

private:
    void on_ssl_info(std::int32_t type, std::int32_t value);
    bool on_verify(std::int32_t ok
                   , x509_store_ctx_st* cert);
};

}


#endif // SSL_ADAPTER_H
