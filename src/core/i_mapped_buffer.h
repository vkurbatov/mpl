#ifndef MPL_I_MAPPED_BUFFER_H
#define MPL_I_MAPPED_BUFFER_H

#include "i_buffer.h"

namespace mpl
{

class i_mapped_buffer : public i_buffer
{
public:
    using u_ptr_t = std::unique_ptr<i_mapped_buffer>;
    using s_ptr_t = std::shared_ptr<i_mapped_buffer>;

    virtual void* map() = 0;
    virtual void make_shared() = 0;
};

}

#endif // MPL_I_MAPPED_BUFFER_H
