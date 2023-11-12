#include "cyclic_data_writer.h"
#include <cstring>

namespace mpl
{


cyclic_buffer_writer::cyclic_buffer_writer(void *data_ref
                                           , std::size_t size_ref)
    : m_data_ref(data_ref)
    , m_size_ref(size_ref)
{

}


bool cyclic_buffer_writer::write(std::uint64_t pos
                                 , const void *data
                                 , std::size_t size)
{
    if (size > 0 && size <= m_size_ref)
    {
        pos = pos % m_size_ref;

        auto part_size = (pos + size) > m_size_ref
                ? m_size_ref - pos
                : size;

        std::memcpy(static_cast<std::int8_t*>(m_data_ref) + pos
                    , data
                    , part_size);

        if (part_size < size)
        {
            std::memcpy(m_data_ref
                        , static_cast<const std::uint8_t*>(data) + part_size
                        , size - part_size);
        }

        return true;
    }

    return false;
}

}
