#ifndef MPL_I_BUFFER_H
#define MPL_I_BUFFER_H

#include "i_data_object.h"

namespace mpl
{

class i_buffer : public i_data_object
{
public:
    using u_ptr_t = std::unique_ptr<i_buffer>;
    using s_ptr_t = std::shared_ptr<i_buffer>;
    virtual u_ptr_t clone() const = 0;
    virtual std::size_t refs() const = 0;
};

}

#endif // MPL_I_BUFFER_H
