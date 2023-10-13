#ifndef SSL_CONST_SSL_MESSAGE_H
#define SSL_CONST_SSL_MESSAGE_H

#include "i_ssl_message.h"

namespace ssl
{

class const_ssl_message : public i_ssl_message
{
    const void*         m_data;
    std::size_t         m_size;
    ssl_data_type_t     m_type;

public:
    const_ssl_message(const void* data
                      , std::size_t size
                      , ssl_data_type_t type);

    // i_ssl_message interface
public:
    const void *data() const override;
    std::size_t size() const override;
    ssl_data_type_t type() const override;
};

}

#endif // SSL_CONST_SSL_MESSAGE_H
