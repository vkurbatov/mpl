#ifndef SSL_DYNAMIC_SSL_MESSAGE_H
#define SSL_DYNAMIC_SSL_MESSAGE_H

#include "i_ssl_message.h"
#include <vector>

namespace ssl
{


class dynamic_ssl_message : public i_dynamic_ssl_message
{
public:
    using buffer_data_t = std::vector<std::uint8_t>;
private:
    buffer_data_t               m_buffer;
    ssl_data_type_t             m_type;

public:
    dynamic_ssl_message(const void* data, std::size_t size
                        , ssl_data_type_t type);
    dynamic_ssl_message(buffer_data_t&& buffer
                        , ssl_data_type_t type);
    dynamic_ssl_message(const i_ssl_message& message);

    buffer_data_t release();

    // i_ssl_message interface
public:
    const void *data() const override;
    std::size_t size() const override;
    ssl_data_type_t type() const override;

    // i_dynamic_ssl_message interface
public:
    void *data() override;
    i_dynamic_ssl_message &append_data(const void *data, std::size_t size) override;
    i_dynamic_ssl_message &resize(std::size_t size, uint8_t fill_data) override;
};

}

#endif // SSL_DYNAMIC_SSL_MESSAGE_H
