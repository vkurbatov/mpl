#ifndef SSL_BIO_CONTEXT_H
#define SSL_BIO_CONTEXT_H

#include "ssl_native.h"
#include <cstdint>
#include <vector>
#include <functional>

namespace ssl
{

using bio_buffer_t = std::vector<std::uint8_t>;

enum class bio_ctrl_id_t
{
    undefined = -1,
    reset = 1,
    eof = 2,
    info = 3,
    get_close = 8,
    set_close = 9,
    pending = 10,
    flush = 11,
    write_pending = 13,
    set_callback = 14,
    get_callback = 15,
    set = 4,
    get = 5,
    push = 6,
    pop = 7,
    dup = 12,
    set_filename = 30,
    dgram_query_mtu = 40,
    dgram_set_mtu = 42,
    dgram_mtu_exceeded = 43,
    dgram_get_peer = 46,
    dgram_get_fallback_mtu = 47
};

class bio_context
{
public:
    using read_handler_t = std::function<std::int32_t(void* data, std::size_t size)>;
    using write_handler_t = std::function<std::int32_t(const void* data, std::size_t size)>;
    using ctrl_handler_t = std::function<std::int32_t(bio_ctrl_id_t cmd, long num, void* ptr)>;

    struct bio_data_info_t
    {
        void*       data = nullptr;
        std::size_t size = 0u;
    };
private:
    bio_ptr_t           m_bio;

    read_handler_t      m_read_handler;
    write_handler_t     m_write_handler;
    ctrl_handler_t      m_ctrl_handler;

public:
    template<typename O>
    static const O* get_native_object();
    static std::size_t read_from_file(bio_ptr_t& bio, const std::string& file_name);
    static std::size_t read(bio_ptr_t& bio, void* data, std::size_t size);
    static std::size_t read(bio_ptr_t& bio, bio_buffer_t& buffer);
    static std::size_t write_to_file(bio_ptr_t& bio, const std::string& file_name, bool append = false);
    static std::size_t write(bio_ptr_t& bio, const void* data, std::size_t size);
    static std::size_t write(bio_ptr_t& bio, const bio_buffer_t& bio_buffer);
    static bool get_data_info(bio_ptr_t& bio, bio_data_info_t& data_info);

    static bool write_x509(bio_ptr_t& bio, const x509_ptr_t& x509);
    static bool write_evp_pkey(bio_ptr_t& bio, const evp_pkey_ptr_t& evp_pkey);

    static x509_ptr_t read_x509(bio_ptr_t& bio);
    static evp_pkey_ptr_t read_evp_pkey(bio_ptr_t& bio);

    static bool set_close(bio_ptr_t& bio, bool close);
    static bool flush(bio_ptr_t& bio);
    static bool reset(bio_ptr_t& bio);

    static bio_ptr_t create_bio();
    static bio_ptr_t create_bio(const std::string& file_name);
    static bio_ptr_t create_bio(const void* data, std::size_t size);

    bio_context();
    bio_context(bio_ptr_t&& bio);
    bio_context(const void* data, std::size_t size);
    bio_context(const std::string& file_name);
    bio_context(read_handler_t read_handler
                , write_handler_t write_handler
                , ctrl_handler_t ctrl_handler);
    ~bio_context();

    std::size_t read_from_file(const std::string& file_name);
    std::size_t write_to_file(const std::string& file_name, bool append = false);

    std::size_t read(void* data, std::size_t size);
    std::size_t write(const void* data, std::size_t size);

    std::size_t read(bio_buffer_t& buffer);
    std::size_t write(const bio_buffer_t& buffer);

    bool write_x509(const x509_ptr_t& x509);
    bool write_evp_pkey(const evp_pkey_ptr_t& evp_pkey);

    x509_ptr_t read_x509();
    evp_pkey_ptr_t read_evp_pkey();

    bool flush();
    bool reset();

    std::size_t read_pending() const;
    std::size_t write_pending() const;
    std::size_t read_count() const;
    std::size_t write_count() const;
    bool get_data_info(bio_data_info_t& data_info);

    void* data();
    const void* data() const;

    bool is_eof() const;

    const bio_ptr_t& native_handle() const;
    bio_ptr_t release_handle();
    bool is_valid() const;
private:
    std::int32_t on_write(const void* data, std::size_t size);
    std::int32_t on_read(void* data, std::size_t size);
    std::int64_t on_ctrl(bio_ctrl_id_t cmd, long num, void* ptr);
};

}

#endif // SSL_BIO_CONTEXT_H
