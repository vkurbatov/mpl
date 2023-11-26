#ifndef MPL_FIFO_WRITER_IMPL_H
#define MPL_FIFO_WRITER_IMPL_H

#include "core/i_fifo_buffer_writer.h"
#include "core/i_shared_data.h"

namespace mpl
{

class fifo_writer_impl : public i_fifo_buffer_writer
{
    i_shared_data&  m_shared_data;
public:
    fifo_writer_impl(i_shared_data& shared_data);

    bool is_valid() const;

    // i_fifo_buffer interface
public:
    void reset() override;
    std::size_t capacity() const override;

    // i_fifo_buffer_writer interface
public:
    bool push_data(const void *data
                   , std::size_t size) override;
private:
    void internal_reset();


};

}

#endif // MPL_FIFO_WRITER_IMPL_H
