#ifndef MPL_I_BUFFER_FACTORY_H
#define MPL_I_BUFFER_FACTORY_H

#include "i_buffer.h"

namespace mpl
{

class i_buffer_factory
{
public:
    using u_ptr_t = std::unique_ptr<i_buffer_factory>;
    using s_ptr_t = std::shared_ptr<i_buffer_factory>;

    virtual ~i_buffer_factory() = default;

    virtual i_buffer::u_ptr_t create_buffer(const void* data
                                            , std::size_t size
                                             , bool copy) = 0;


};

}

#endif // MPL_I_BUFFER_FACTORY_H
