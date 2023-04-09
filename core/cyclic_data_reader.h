#ifndef MPL_CYCLIC_DATA_READER_H
#define MPL_CYCLIC_DATA_READER_H

#include <cstdint>

namespace mpl
{

class cyclic_data_reader
{
    const void*     m_data_ref;
    std::size_t     m_size_ref;
public:
    cyclic_data_reader(const void* data_ref
                       , std::size_t size_ref);

    bool read(std::size_t pos
              , void* data
              , std::size_t size);
};

}

#endif // MPL_CYCLIC_DATA_READER_H
