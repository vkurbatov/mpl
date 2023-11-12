#ifndef MPL_I_DYNAMIC_BUFFER_H
#define MPL_I_DYNAMIC_BUFFER_H

#include "i_mapped_buffer.h"

namespace mpl
{

class i_dynamic_buffer : public i_mapped_buffer
{
public:
    using u_ptr_t = std::unique_ptr<i_dynamic_buffer>;
    using s_ptr_t = std::shared_ptr<i_dynamic_buffer>;
    virtual bool append_data(const void* data
                             , std::size_t size) = 0;
    virtual void resize(std::size_t new_size) = 0;
    virtual void clear() = 0;
};

}

#endif // MPL_I_DYNAMIC_BUFFER_H
