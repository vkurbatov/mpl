#include "buffer_factory_impl.h"
#include "smart_buffer.h"

namespace mpl::utils
{

buffer_factory_impl::u_ptr_t buffer_factory_impl::create()
{
    return std::make_unique<buffer_factory_impl>();
}

buffer_factory_impl::buffer_factory_impl()
{

}

i_buffer::u_ptr_t buffer_factory_impl::create_buffer(const void *data
                                                     , std::size_t size
                                                     , bool copy)
{
    return smart_buffer::create(data
                                , size
                                , copy);
}


}
