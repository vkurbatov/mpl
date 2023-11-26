#ifndef MPL_I_SHARED_DATA_H
#define MPL_I_SHARED_DATA_H

#include <memory>

namespace mpl
{

class i_shared_data
{
public:
    using u_ptr_t = std::unique_ptr<i_shared_data>;
    using s_ptr_t = std::shared_ptr<i_shared_data>;

    virtual ~i_shared_data() = default;
    virtual const void* map() const = 0;
    virtual void* map() = 0;
    virtual bool unmap() const = 0;
    virtual bool unmap() = 0;
    virtual std::size_t size() const = 0;
};

}

#endif // MPL_I_SHARED_DATA_H
