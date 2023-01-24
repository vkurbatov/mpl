#include "data_splitter.h"
#include <cstring>

namespace mpl
{

data_splitter::data_splitter(std::size_t fragment_size)
{
    reset(fragment_size);
}

void data_splitter::reset(std::size_t fragment_size)
{
    m_work_fragment.clear();
    if (fragment_size > 0)
    {
        m_work_fragment.reserve(fragment_size);
    }
}

data_fragment_queue_t data_splitter::push_stream(const void *data
                                                 , std::size_t size)
{
    data_fragment_queue_t queue;
    auto data_ptr = static_cast<const std::uint8_t*>(data);
    auto fragment_size = m_work_fragment.capacity();

    if (fragment_size > 0)
    {

        while(size > 0)
        {
            std::size_t process_data = 0;

            if (m_work_fragment.size() < fragment_size)
            {
                process_data = fragment_size - m_work_fragment.size();
                process_data = std::min(process_data, size);
                auto idx = m_work_fragment.size();

                m_work_fragment.resize(idx + process_data);
                std::memcpy(m_work_fragment.data() + idx, data_ptr, process_data);
            }

            if (m_work_fragment.size() == fragment_size)
            {
                queue.emplace(std::move(m_work_fragment));
                m_work_fragment.reserve(fragment_size);
            }

            data_ptr += process_data;
            size -= process_data;
        }

    }

    return queue;
}

std::size_t data_splitter::fragment_size() const
{
    return m_work_fragment.capacity();
}

std::size_t data_splitter::buffered_size() const
{
    return m_work_fragment.size();
}

}
