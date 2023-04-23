#ifndef MPL_FIFO_READER_IMPL_H
#define MPL_FIFO_READER_IMPL_H

#include "i_fifo_buffer_reader.h"
#include "i_shared_data.h"
#include <cstdint>

namespace mpl
{

class fifo_reader_impl : public i_fifo_buffer_reader
{
    const i_shared_data&        m_shared_data;
    std::uint64_t               m_position;
public:
    fifo_reader_impl(const i_shared_data& shared_data);

    // i_fifo_buffer interface
public:
    void reset() override;
    std::size_t capacity() const override;

    // i_fifo_buffer_reader interface
public:
    std::size_t read_data(void *data, std::size_t size) const override;
    std::size_t pop_data(void *data, std::size_t size) override;
    std::size_t pending_size() const override;

private:
    std::size_t internal_capacity() const;
    void internal_reset();
    std::size_t internal_read_data(void *data, std::size_t size) const;
};

}

#endif // MPL_FIFO_READER_IMPL_H
