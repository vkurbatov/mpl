#ifndef MPL_DATA_SPLITTER_H
#define MPL_DATA_SPLITTER_H

#include <vector>
#include <queue>
#include <cstdint>

namespace mpl
{

using data_fragment_t = std::vector<std::uint8_t>;
using data_fragment_queue_t = std::queue<data_fragment_t>;

class data_splitter
{
    data_fragment_t         m_work_fragment;
public:
    data_splitter(std::size_t fragment_size);
    void reset(std::size_t fragment_size = 0);
    data_fragment_queue_t push_stream(const void* data
                                      , std::size_t size);
    std::size_t fragment_size() const;
    std::size_t buffered_size() const;

};

}


#endif // MPL_DATA_SPLITTER_H
