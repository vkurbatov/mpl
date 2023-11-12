#include "cyclic_data_reader.h"
#include <cstring>

namespace mpl
{

cyclic_data_reader::cyclic_data_reader(const void *data_ref
                                       , std::size_t size_ref)
    : m_data_ref(data_ref)
    , m_size_ref(size_ref)
{

}

bool cyclic_data_reader::read(std::size_t pos
                              , void* data
                              , std::size_t size)
{
    if (size > 0 && size < m_size_ref)
    {
        pos = pos % m_size_ref;

        auto part_size = (pos + size) > m_size_ref
                ? m_size_ref - pos
                : size;


        std::memcpy(data
                   , static_cast<const std::uint8_t*>(m_data_ref) + pos
                   , part_size);

        if (part_size < size)
        {

            std::memcpy(static_cast<std::uint8_t*>(data) + part_size
                        , m_data_ref
                        , size - part_size);
        }

        return true;
    }

    return false;
}


}
