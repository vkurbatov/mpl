#include "data_container.h"

namespace portable
{

data_container::data_container(data_handler_t &&data_handler)
    : m_data_handler(std::move(data_handler))
{

}

bool data_container::get_buffer(std::int32_t buffer_index, data_descriptor_t& deta_desc) const
{
    return m_data_handler != nullptr
            && m_data_handler(buffer_index, deta_desc);
}

data_desc_list_t data_container::get_buffers() const
{
    data_desc_list_t desc_data_list;
    data_descriptor_t data_desc;

    while(get_buffer(desc_data_list.size(), data_desc))
    {
        desc_data_list.push_back(data_desc);
    }

    return desc_data_list;
}


std::size_t data_container::size() const
{
    std::size_t result = 0;
    data_descriptor_t data_desc;
    std::int32_t idx = 0;

    while(get_buffer(idx, data_desc))
    {
        result += data_desc.size;
        idx++;
    }

    return result;
}


}
