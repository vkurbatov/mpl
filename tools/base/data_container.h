#ifndef BASE_DATA_CONTAINER_H
#define BASE_DATA_CONTAINER_H

#include <functional>
#include <vector>
#include <cstdint>

namespace portable
{

using data_ptr_t = void*;

struct data_descriptor_t
{
    const void* data = nullptr;
    std::size_t size = 0;
};

using data_desc_list_t = std::vector<data_descriptor_t>;

using data_handler_t = std::function<bool(std::int32_t buffer_index, data_descriptor_t& data_descriptor)>;

class data_container
{
    data_handler_t          m_data_handler;
public:
    data_container(data_handler_t&& data_handler);
    bool get_buffer(std::int32_t buffer_index, data_descriptor_t& deta_desc) const;
    data_desc_list_t get_buffers() const;
    std::size_t size() const;
};

}

#endif // DATA_CONTAINER_H
