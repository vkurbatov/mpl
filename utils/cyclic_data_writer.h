#ifndef MPL_CYCLIC_DATA_WRITER_H
#define MPL_CYCLIC_DATA_WRITER_H

#include <cstdint>

namespace mpl
{

class cyclic_buffer_writer
{
    void*           m_data_ref;
    std::size_t     m_size_ref;
public:

    cyclic_buffer_writer(void* data_ref
                         , std::size_t size_ref);

    bool write(std::uint64_t pos
               , const void* data
               , std::size_t size);
};

}

#endif // MPL_CYCLIC_DATA_WRITER_H
