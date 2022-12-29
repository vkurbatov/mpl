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

smart_buffer *smart_buffer_collection::get_buffer(int32_t index)
{
    if (auto it = m_buffers.find(index); it != m_buffers.end())
    {
        return &it->second;
    }

    return nullptr;
}

const smart_buffer *smart_buffer_collection::get_buffer(int32_t index) const
{
    if (auto it = m_buffers.find(index); it != m_buffers.end())
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

std::size_t smart_buffer_collection::count() const
{
    return m_buffers.size();
}

std::vector<int32_t> smart_buffer_collection::index_list() const
{
    std::vector<int32_t> result;
    for (const auto& b : m_buffers)
    {
        result.push_back(b.first);
    }

    return result;
}

smart_buffer_collection smart_buffer_collection::fork() const
{
    smart_buffer_collection foked_collection(*this);

    foked_collection.make_shared();

    return foked_collection;
}




}
