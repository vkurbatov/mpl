#ifndef MPL_BUFFER_FACTORY_IMPL_H
#define MPL_BUFFER_FACTORY_IMPL_H

#include "core/i_buffer_factory.h"

namespace mpl::utils
{

class buffer_factory_impl : public i_buffer_factory
{
public:

    using u_ptr_t = std::unique_ptr<buffer_factory_impl>;
    using s_ptr_t = std::shared_ptr<buffer_factory_impl>;

    static u_ptr_t create();

    buffer_factory_impl();

    // i_buffer_factory interface
public:
    i_buffer::u_ptr_t create_buffer(const void *data
                                    , std::size_t size
                                    , bool copy) override;
};

}

#endif // MPL_BUFFER_FACTORY_IMPL_H
