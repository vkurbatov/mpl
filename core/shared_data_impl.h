#ifndef MPL_SHARED_DATA_IMPL_H
#define MPL_SHARED_DATA_IMPL_H

#include "i_shared_data.h"
#include "tools/base/sync_base.h"

namespace mpl
{

class shared_data_ref_impl : public i_shared_data
{
    using mutex_t = base::shared_spin_lock;

    mutable mutex_t     m_safe_mutex;

    void*               m_data_ref;
    std::size_t         m_size_ref;
public:
    shared_data_ref_impl(void* data_ref
                         , std::size_t size_ref);

    // i_shared_data interface
public:
    const void *map() const override;
    void *map() override;
    bool unmap() const override;
    bool unmap() override;
    std::size_t size() const override;
};

}

#endif // MPL_SHARED_DATA_IMPL_H
