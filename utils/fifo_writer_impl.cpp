#include "fifo_writer_impl.h"
#include "shared_mutex"
#include "utils/shared_buffer_header.h"
#include "cyclic_data_writer.h"
#include "utils/time_utils.h"

namespace mpl
{

fifo_writer_impl::fifo_writer_impl(i_shared_data& shared_data)
    : m_shared_data(shared_data)
{
    internal_reset();
}

void fifo_writer_impl::reset()
{
    internal_reset();
}

std::size_t fifo_writer_impl::capacity() const
{
    auto size = m_shared_data.size();
    return size > sizeof(shared_buffer_header_t)
            ? size - sizeof(shared_buffer_header_t)
            : 0u;
}

bool fifo_writer_impl::push_data(const void *data
                                 , std::size_t size)
{
    bool result = false;

    if (auto mapped_data = m_shared_data.map())
    {
        auto& header = *static_cast<shared_buffer_header_t*>(mapped_data);
        auto buffer_data = static_cast<std::uint8_t*>(mapped_data) + sizeof(shared_buffer_header_t);
        auto buffer_size = m_shared_data.size() - sizeof(shared_buffer_header_t);


        cyclic_buffer_writer writer(buffer_data
                                    , buffer_size);

        if (writer.write(header.position
                         , data
                         , size))
        {
            header.position += size;
            header.state.packets ++;
            header.timestamp = core::utils::now();

            result = true;
        }

        m_shared_data.unmap();
    }

    return result;
}

void fifo_writer_impl::internal_reset()
{
    if (auto data = m_shared_data.map())
    {
        auto& header = *static_cast<shared_buffer_header_t*>(data);

        header.reset();

        m_shared_data.unmap();
    }
}

}
