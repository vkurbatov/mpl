#include "bio_context.h"
#include "ssl_native.h"
#include "bio_config.h"
#include "ssl_utils.h"

#include <openssl/ssl.h>
#include <fstream>

#include <cstring>
#include <iostream>

namespace ssl
{

static constexpr std::size_t max_chunk_size = 1024;

template<>
const BIO_METHOD* bio_context::get_native_object()
{
    static BIO_METHOD* methods = []
    {
        BIO_METHOD* methods = BIO_meth_new(BIO_TYPE_BIO, "custom");

        static auto custom_write = [](BIO* b, const char* data, std::int32_t size) -> std::int32_t
        {
            if (b != nullptr)
            {
                if (auto ctx = reinterpret_cast<bio_context*>(BIO_get_data(b)))
                {
                    return ctx->on_write(data, size);
                }
            }

            return -1;
        };

        static auto custom_read = [](BIO* b, char* data, std::int32_t size) -> std::int32_t
        {
            if (b != nullptr)
            {
                if (auto ctx = reinterpret_cast<bio_context*>(BIO_get_data(b)))
                {
                    return ctx->on_read(data, size);
                }
            }

            return -1;
        };

        static auto custom_puts = [&custom_write](BIO* b, const char* data) -> std::int32_t
        {
            return custom_write(b
                                , data
                                , std::strlen(data));
        };


        static auto custom_ctrl = [](BIO* b, std::int32_t cmd, std::int64_t num, void* ptr) -> std::int64_t
        {
            if (b != nullptr)
            {
                if (auto ctx = reinterpret_cast<bio_context*>(BIO_get_data(b)))
                {
                    return ctx->on_ctrl(static_cast<bio_ctrl_id_t>(cmd)
                                        , num
                                        , ptr);
                }
            }

            return -1;
        };

        static auto custom_create = [](BIO* b) -> std::int32_t
        {
            BIO_set_shutdown(b, 0);
            BIO_set_init(b, 1);
            BIO_set_data(b, nullptr);
            if (b != nullptr)
            {
                BIO_set_shutdown(b, 0);
                BIO_set_init(b, 1);
                BIO_set_data(b, nullptr);
                return 1;
            }

            return 0;
        };

        static auto custom_destroy = [](BIO* b) -> std::int32_t
        {
            if (b == nullptr)
            {
              return 0;
            }
            return 1;
        };

        BIO_meth_set_write(methods, custom_write);
        BIO_meth_set_read(methods, custom_read);
        BIO_meth_set_puts(methods, custom_puts);
        BIO_meth_set_ctrl(methods, custom_ctrl);
        BIO_meth_set_create(methods, custom_create);
        BIO_meth_set_destroy(methods, custom_destroy);
        return methods;
    }();
    return methods;
}

std::size_t bio_context::read_from_file(bio_ptr_t &bio, const std::string &file_name)
{
    std::size_t result = 0;

    std::ifstream file(file_name, std::ifstream::binary);

    if (file.is_open())
    {
        static thread_local char buffer[max_chunk_size];
        while(!file.eof())
        {
            file.read(buffer, sizeof(buffer));
            auto cnt = file.gcount();
            result += write(bio, buffer, cnt);
        }
    }

    return result;
}

std::size_t bio_context::read(bio_ptr_t &bio, void *data, std::size_t size)
{
    return BIO_read(bio.get(), data, size);
}

std::size_t bio_context::read(bio_ptr_t &bio, bio_buffer_t &buffer)
{
    bio_data_info_t data_info;
    if (get_data_info(bio
                      , data_info))
    {
        buffer.insert(buffer.end()
                      , static_cast<const bio_buffer_t::value_type*>(data_info.data)
                      , static_cast<const bio_buffer_t::value_type*>(data_info.data) + data_info.size);
    }

    return 0;
}

std::size_t bio_context::write_to_file(bio_ptr_t &bio
                                , const std::string &file_name
                                , bool append)
{
    std::size_t result = 0;

    auto mode = std::ifstream::binary;
    if (append)
    {
        mode |= std::ifstream::app;
    }

    std::ofstream ofile(file_name, mode);
    if (ofile.is_open())
    {
        BUF_MEM *buf = nullptr;
        if (BIO_get_mem_ptr(bio.get(), &buf) > 0)
        {
            if (ofile.write(buf->data, buf->length))
            {
                result += buf->length;
            }
        }
    }

    return result;
}

std::size_t bio_context::write(bio_ptr_t &bio, const void *data, std::size_t size)
{
    return BIO_write(bio.get(), data , size);
}

std::size_t bio_context::write(bio_ptr_t& bio
                               , const bio_buffer_t &bio_buffer)
{
    return write(bio
                 , bio_buffer.data()
                 , bio_buffer.size());
}

bool bio_context::get_data_info(bio_ptr_t &bio, bio_context::bio_data_info_t &data_info)
{
    BUF_MEM *buf = nullptr;
    if (BIO_get_mem_ptr(bio.get(), &buf) > 0)
    {
        data_info.data = buf->data;
        data_info.size = buf->length;
        return true;
    }
    return false;
}

bool bio_context::write_x509(bio_ptr_t &bio
                             , const x509_ptr_t &x509)
{
    return PEM_write_bio_X509(bio.get(), x509.get()) != 0;
}

bool bio_context::write_evp_pkey(bio_ptr_t &bio
                                 , const evp_pkey_ptr_t &evp_pkey)
{
    return PEM_write_bio_PrivateKey(bio.get()
                                    , evp_pkey.get()
                                    , nullptr
                                    , nullptr
                                    , 0
                                    , nullptr
                                    , nullptr) != 0;
}

x509_ptr_t bio_context::read_x509(bio_ptr_t &bio)
{
    return create_object<X509, const bio_ptr_t&>(bio);
}

evp_pkey_ptr_t bio_context::read_evp_pkey(bio_ptr_t &bio)
{
    return create_object<EVP_PKEY, const bio_ptr_t&>(bio);
}

bool bio_context::set_close(bio_ptr_t &bio, bool close)
{
    return BIO_set_close(bio.get(), close ? BIO_CLOSE : BIO_NOCLOSE) > 0;
}

bool bio_context::flush(bio_ptr_t &bio)
{
    return BIO_flush(bio.get()) != 0;
}

bool bio_context::reset(bio_ptr_t &bio)
{
    return BIO_reset(bio.get()) != 0;
}


bio_ptr_t bio_context::create_bio()
{
    return create_object<BIO>(bio_method_t::in_memory_method);
}

bio_ptr_t bio_context::create_bio(const std::string &file_name)
{
    if (auto bio = create_bio())
    {
        if (read_from_file(bio, file_name) > 0)
        {
            return bio;
        }
    }

    return nullptr;
}

bio_ptr_t bio_context::create_bio(const void *data
                                  , std::size_t size)
{
    if (auto bio = create_bio())
    {
        if (write(bio, data, size) > 0)
        {
            return bio;
        }
    }

    return nullptr;
}

bio_context::bio_context()
    : bio_context(create_bio())
{

}


bio_context::bio_context(bio_ptr_t&& bio)
    : m_bio(std::move(bio))
{
    BIO_set_retry_read(m_bio.get());
    BIO_set_retry_write(m_bio.get());
}

bio_context::bio_context(const void *data
                         , std::size_t size)
    : bio_context(create_bio(data
                              , size))
{

}

bio_context::bio_context(const std::string &file_name)
    : bio_context(create_bio(file_name))
{

}

bio_context::bio_context(bio_context::read_handler_t read_handler
                         , bio_context::write_handler_t write_handler
                         , bio_context::ctrl_handler_t ctrl_handler)
    : bio_context(create_object<BIO>(get_native_object<BIO_METHOD>()))
{
    m_read_handler = std::move(read_handler);
    m_write_handler = std::move(write_handler);
    m_ctrl_handler = std::move(ctrl_handler);
    BIO_set_data(m_bio.get()
                 , this);
}

bio_context::~bio_context()
{

}

std::size_t bio_context::read_from_file(const std::string &file_name)
{
    return read_from_file(m_bio
                          , file_name);
}

std::size_t bio_context::write_to_file(const std::string &file_name, bool append)
{
    return write_to_file(m_bio
                         , file_name
                         , append);
}

std::size_t bio_context::read(void *data, std::size_t size)
{
    return read(m_bio, data, size);
}

std::size_t bio_context::write(const void *data, std::size_t size)
{
    return write(m_bio, data, size);
}

std::size_t bio_context::read(bio_buffer_t &buffer)
{
    return read(m_bio
                , buffer);
}

std::size_t bio_context::write(const bio_buffer_t &buffer)
{
    return write(m_bio
                , buffer);
}

bool bio_context::write_x509(const x509_ptr_t &x509)
{
    return write_x509(m_bio, x509);
}

bool bio_context::write_evp_pkey(const evp_pkey_ptr_t &evp_pkey)
{
    return write_evp_pkey(m_bio, evp_pkey);
}

x509_ptr_t bio_context::read_x509()
{
    return read_x509(m_bio);
}

evp_pkey_ptr_t bio_context::read_evp_pkey()
{
    return read_evp_pkey(m_bio);
}

bool bio_context::flush()
{
    return flush(m_bio) > 0;
}

bool bio_context::reset()
{
    return reset(m_bio) > 0;
}

std::size_t bio_context::read_pending() const
{
    return BIO_pending(m_bio.get());
}

std::size_t bio_context::write_pending() const
{
    return BIO_wpending(m_bio.get());
}

std::size_t bio_context::read_count() const
{
    return BIO_number_read(m_bio.get());
}

std::size_t bio_context::write_count() const
{
    return BIO_number_written(m_bio.get());
}

bool bio_context::get_data_info(bio_context::bio_data_info_t &data_info)
{
    return get_data_info(m_bio
                         , data_info);
}

void *bio_context::data()
{
    char* data = nullptr;
    if (BIO_get_mem_data(m_bio.get(), &data) >= 0)
    {
        return data;
    }

    return nullptr;
}

const void *bio_context::data() const
{
    char* data = nullptr;
    if (BIO_get_mem_data(m_bio.get(), &data) >= 0)
    {
        return data;
    }

    return nullptr;
}

bool bio_context::is_eof() const
{
    return BIO_eof(m_bio.get()) != 0;
}

const bio_ptr_t& bio_context::native_handle() const
{
    return m_bio;
}

bio_ptr_t bio_context::release_handle()
{
    return std::move(m_bio);
}

bool bio_context::is_valid() const
{
    return m_bio != nullptr;
}

int32_t bio_context::on_write(const void *data
                              , std::size_t size)
{
    if (m_write_handler != nullptr)
    {
        return m_write_handler(data
                               , size);
    }
    return -1;
}

int32_t bio_context::on_read(void *data
                             , std::size_t size)
{
    if (m_read_handler != nullptr)
    {
        return m_read_handler(data
                              , size);
    }
    return -1;
}

int64_t bio_context::on_ctrl(bio_ctrl_id_t cmd
                             , long num
                             , void *ptr)
{
    if (m_ctrl_handler != nullptr)
    {
        return m_ctrl_handler(cmd
                              , num
                              , ptr);
    }
    return -1;
}

}
