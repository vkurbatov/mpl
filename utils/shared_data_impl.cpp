#include "shared_data_impl.h"

namespace mpl
{

shared_data_ref_impl::shared_data_ref_impl(void *data_ref
                                           , std::size_t size_ref)
    : m_data_ref(data_ref)
    , m_size_ref(size_ref)
{

}

const void *shared_data_ref_impl::map() const
{
    m_safe_mutex.lock_shared();
    return m_data_ref;
}

void *shared_data_ref_impl::map()
{
    m_safe_mutex.lock();
    return m_data_ref;
}

bool shared_data_ref_impl::unmap() const
{
    m_safe_mutex.unlock_shared();
    return true;
}

bool shared_data_ref_impl::unmap()
{
    m_safe_mutex.unlock();
    return true;
}

std::size_t shared_data_ref_impl::size() const
{
    return m_size_ref;
}



}
