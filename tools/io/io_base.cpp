#include "io_base.h"

namespace io
{

link_config_t::link_config_t()
    : link_config_t(link_type_t::undefined)
{

}

bool link_config_t::is_valid() const
{
    return false;
}

link_config_t::link_config_t(link_type_t type)
    : type(type)
{

}

message_t::message_t(const void *data
                     , std::size_t size
                     , bool store)
    : m_ref_data(nullptr)
    , m_ref_size(0)
{
    assign(data
           , size
           , store);
}

message_t::message_t(raw_array_t &&raw_data)
{
    assign(std::move(raw_data));
}

void message_t::assign(const void *data
                       , std::size_t size
                       , bool store)
{
    if (store)
    {
        m_ref_data = nullptr;
        m_ref_size = 0;
        if (data == nullptr)
        {
            m_store_data.assign(size, 0);
        }
        else
        {
            m_store_data.assign(static_cast<const std::uint8_t*>(data)
                                , static_cast<const std::uint8_t*>(data) + size);
        }
    }
    else
    {
        m_ref_data = data;
        m_ref_size = size;
        m_store_data.clear();
    }
}

void message_t::assign(raw_array_t &&raw_data)
{
    m_ref_data = nullptr;
    m_ref_size = 0;
    m_store_data = std::move(raw_data);
}

raw_array_t message_t::release()
{
    return std::move(m_store_data);
}

void message_t::make_store()
{
    if (!is_stored())
    {
        assign(m_ref_data
               , m_ref_size
               , true);
    }
}

bool message_t::is_stored() const
{
    return m_ref_data == nullptr;
}

const void *message_t::data() const
{
    return is_stored()
            ? m_store_data.data()
            : m_ref_data;
}

void *message_t::map()
{
    make_store();
    return m_store_data.data();
}

std::size_t message_t::size() const
{
    return is_stored()
            ? m_store_data.size()
            : m_ref_size;
}

void message_t::clear()
{
    m_ref_data = nullptr;
    m_ref_size = 0;
    m_store_data.clear();
}

bool message_t::is_empty() const
{
    return size() == 0;
}


}
