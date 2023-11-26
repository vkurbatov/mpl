#ifndef MPL_I_FIFO_BUFFER_H
#define MPL_I_FIFO_BUFFER_H

#include <memory>

namespace mpl
{

class i_fifo_buffer
{
public:
    static const std::size_t overload = static_cast<std::size_t>(-1);
    using u_ptr_t = std::unique_ptr<i_fifo_buffer>;
    using s_ptr_t = std::shared_ptr<i_fifo_buffer>;
    virtual ~i_fifo_buffer() = default;
    virtual void reset() = 0;
    virtual std::size_t capacity() const = 0;

};

}

#endif // MPL_I_FIFO_BUFFER_H
