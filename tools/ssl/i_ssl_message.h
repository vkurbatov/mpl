#ifndef SSL_I_SSL_MESSAGE_H
#define SSL_I_SSL_MESSAGE_H

#include "ssl_types.h"
#include <cstdint>

namespace pt::ssl
{

class i_ssl_message
{
public:
    virtual ~i_ssl_message() = default;
    virtual const void* data() const = 0;
    virtual std::size_t size() const = 0;
    virtual ssl_data_type_t type() const = 0;
};

class i_dynamic_ssl_message : public i_ssl_message
{
public:
    virtual ~i_dynamic_ssl_message() = default;
    virtual void* data() = 0;
    virtual i_dynamic_ssl_message& append_data(const void* data, std::size_t size) = 0;
    virtual i_dynamic_ssl_message& resize(std::size_t size, std::uint8_t fill_data = 0) = 0;
};

}

#endif // SSL_I_SSL_MESSAGE_H
