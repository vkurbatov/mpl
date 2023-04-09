#ifndef MPL_I_FIFO_BUFFER_WRITER_H
#define MPL_I_FIFO_BUFFER_WRITER_H

#include "i_fifo_buffer.h"
#include <vector>

namespace mpl
{

class i_fifo_buffer_writer : public i_fifo_buffer
{
public:
    using u_ptr_t = std::unique_ptr<i_fifo_buffer_writer>;
    using s_ptr_t = std::shared_ptr<i_fifo_buffer_writer>;

    virtual ~i_fifo_buffer_writer() = default;
    virtual bool push_data(const void* data, std::size_t size) = 0;
};

}

#endif // MPL_I_FIFO_BUFFER_WRITER_H
