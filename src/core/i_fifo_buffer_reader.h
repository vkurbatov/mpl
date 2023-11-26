#ifndef MPL_I_FIFO_BUFFER_READER_H
#define MPL_I_FIFO_BUFFER_READER_H

#include "i_fifo_buffer.h"

namespace mpl
{

class i_fifo_buffer_reader : public i_fifo_buffer
{
public:
    using u_ptr_t = std::unique_ptr<i_fifo_buffer_reader>;
    using s_ptr_t = std::shared_ptr<i_fifo_buffer_reader>;

    virtual ~i_fifo_buffer_reader() = default;
    virtual std::size_t read_data(void* data, std::size_t size) const = 0;
    virtual std::size_t pop_data(void* data, std::size_t size) = 0;
    virtual std::size_t pending_size() const = 0;
    virtual std::size_t cursor() const = 0;
    virtual bool seek(std::size_t cursor) = 0;
};

}

#endif // MPL_I_FIFO_BUFFER_READER_H
