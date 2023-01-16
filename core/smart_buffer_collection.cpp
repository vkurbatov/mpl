#include "smart_buffer_collection.h"

namespace mpl
{

smart_buffer_collection::smart_buffer_collection()
{

}

void smart_buffer_collection::set_buffer(int32_t index
                                         , const smart_buffer &buffer)
{
    m_buffers.emplace(index
                      , buffer);
}

void smart_buffer_collection::set_buffer(int32_t index
                                         , smart_buffer &&buffer)
{
    m_buffers.emplace(index
                      , std::move(buffer));
}

bool smart_buffer_collection::remove_buffer(int32_t index)
{
    return m_buffers.erase(index);
}

smart_buffer *smart_buffer_collection::get_smart_buffer(int32_t index)
{
    if (auto it = m_buffers.find(index)
            ; it != m_buffers.end())
    {
        return &it->second;
    }

    return nullptr;
}

const smart_buffer *smart_buffer_collection::get_smart_buffer(int32_t index) const
{
    if (auto it = m_buffers.find(index)
            ; it != m_buffers.end())
    {
        return &it->second;
    }

    return nullptr;
}

void smart_buffer_collection::make_shared()
{
    for (auto& b : m_buffers)
    {
        b.second.make_shared();
    }
}

smart_buffer_collection smart_buffer_collection::fork() const
{
    smart_buffer_collection foked_collection(*this);

    foked_collection.make_shared();

    return foked_collection;
}

void smart_buffer_collection::assign(const i_buffer_collection &other)
{
    m_buffers.clear();
    for (const auto& i : other.index_list())
    {
        if (auto b = other.get_buffer(i))
        {
            m_buffers.emplace(i
                              , b->clone());
        }
    }
}

const i_buffer *smart_buffer_collection::get_buffer(index_t index) const
{
    return get_smart_buffer(index);
}

i_buffer_collection::index_list_t smart_buffer_collection::index_list() const
{
    i_buffer_collection::index_list_t result;
    for (const auto& b : m_buffers)
    {
        result.push_back(b.first);
    }

    return result;
}

std::size_t smart_buffer_collection::count() const
{
    return m_buffers.size();
}



}
