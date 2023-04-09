#include "fifo_reader_impl.h"
#include "shared_buffer_header.h"
#include "cyclic_data_reader.h"

namespace mpl
{

fifo_reader_impl::fifo_reader_impl(const i_shared_data &shared_data)
    : m_shared_data(shared_data)
    , m_position(0)
{
    internal_reset();
}

void fifo_reader_impl::reset()
{
    internal_reset();
}

std::size_t fifo_reader_impl::capacity() const
{
    return internal_capacity();
}

bool fifo_reader_impl::read_data(void *data, std::size_t size) const
{
    return internal_read_data(data, size);
}

bool fifo_reader_impl::pop_data(void *data, std::size_t size)
{
    if (internal_read_data(data, size))
    {
        m_position += size;
        return true;
    }

    return false;
}

std::size_t fifo_reader_impl::pending_size() const
{
    std::size_t result = override_size;
    if (auto mapped_data = m_shared_data.map())
    {

        const shared_buffer_header_t& header = *static_cast<const shared_buffer_header_t*>(mapped_data);
        auto pending = header.position - m_position;
        auto capacity = internal_capacity();

        if (pending < capacity)
        {
            result = pending;
        }

        m_shared_data.unmap();
    }
    return result;
}

std::size_t fifo_reader_impl::internal_capacity() const
{
    auto size = m_shared_data.size();
    return size > sizeof(m_shared_data)
            ? size - sizeof(m_shared_data)
            : 0u;
}

void fifo_reader_impl::internal_reset()
{
    if (auto mapped_data = m_shared_data.map())
    {
        m_position = static_cast<const shared_buffer_header_t*>(mapped_data)->position;
        m_shared_data.unmap();
    }
}

bool fifo_reader_impl::internal_read_data(void *data
                                          , std::size_t size) const
{
    bool result = false;

    if (auto mapped_data = m_shared_data.map())
    {

        const shared_buffer_header_t& header = *static_cast<const shared_buffer_header_t*>(mapped_data);
        auto buffer_data = static_cast<const std::uint8_t*>(mapped_data) + sizeof(shared_buffer_header_t);
        auto buffer_size = m_shared_data.size() - sizeof(shared_buffer_header_t);
        auto unread_size =  header.position - m_position;

        if (unread_size < buffer_size)
        {
            cyclic_data_reader reader(buffer_data
                                      , buffer_size);

            if (reader.read(m_position
                            , data
                            , size))
            {
                result = true;
            }
        }

        m_shared_data.unmap();

    }

    return result;
}



}