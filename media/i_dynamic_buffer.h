#ifndef MPL_I_DYNAMIC_BUFFER_H
#define MPL_I_DYNAMIC_BUFFER_H

#include "i_buffer.h"

namespace mpl
{

class i_dynamic_buffer : public i_buffer
{
public:
    using s_ptr_t = std::shared_ptr<i_dynamic_buffer>;
    virtual void append_data(const void* data
                             , std::size_t size) = 0;
    virtual void resize(std::size_t new_size) = 0;
    virtual void clear() = 0;
};

}

#endif // MPL_I_DYNAMIC_BUFFER_H
