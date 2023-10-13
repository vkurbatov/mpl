#include "dynamic_ssl_message.h"

namespace ssl
{

dynamic_ssl_message::dynamic_ssl_message(const void *data
                                         , std::size_t size
                                         , ssl_data_type_t type)
    : m_buffer(static_cast<const buffer_data_t::value_type*>(data)
             , static_cast<const buffer_data_t::value_type*>(data) + size)
    , m_type(type)
{

}

dynamic_ssl_message::dynamic_ssl_message(dynamic_ssl_message::buffer_data_t &&buffer
                                         , ssl_data_type_t type)
    : m_buffer(std::move(m_buffer))
    , m_type(type)
{

}

dynamic_ssl_message::dynamic_ssl_message(const i_ssl_message &message)
    : dynamic_ssl_message(message.data()
                          , message.size()
                          , message.type())
{

}

dynamic_ssl_message::buffer_data_t dynamic_ssl_message::release()
{
    return std::move(m_buffer);
}

const void *dynamic_ssl_message::data() const
{
    return m_buffer.data();
}

std::size_t dynamic_ssl_message::size() const
{
    return m_buffer.size();
}

ssl_data_type_t dynamic_ssl_message::type() const
{
    return m_type;
}

void *dynamic_ssl_message::data()
{
    return m_buffer.data();
}

i_dynamic_ssl_message &dynamic_ssl_message::append_data(const void *data, std::size_t size)
{
    m_buffer.insert(m_buffer.end()
                    , static_cast<const buffer_data_t::value_type*>(data)
                    , static_cast<const buffer_data_t::value_type*>(data) + size);
    return *this;
}

i_dynamic_ssl_message &dynamic_ssl_message::resize(std::size_t size, uint8_t fill_data)
{
    m_buffer.resize(size, fill_data);
    return *this;
}

}
