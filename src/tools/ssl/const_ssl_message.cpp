#include "const_ssl_message.h"

namespace pt::ssl
{

const_ssl_message::const_ssl_message(const void *data
                                     , std::size_t size
                                     , ssl_data_type_t type)
    : m_data(data)
    , m_size(size)
    , m_type(type)
{

}

const void *const_ssl_message::data() const
{
    return m_data;
}

std::size_t const_ssl_message::size() const
{
    return m_size;
}

ssl_data_type_t const_ssl_message::type() const
{
    return m_type;
}

}
