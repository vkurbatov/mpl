#ifndef MPL_I_DATA_OBJECT_H
#define MPL_I_DATA_OBJECT_H

#include <cstdint>
#include <memory>

namespace mpl
{

class i_data_object
{
public:
    using u_ptr_t = std::unique_ptr<i_data_object>;
    using s_ptr_t = std::shared_ptr<i_data_object>;
    using w_ptr_t = std::weak_ptr<i_data_object>;
    virtual ~i_data_object() = default;
    virtual const void* data() const = 0;
    virtual std::size_t size() const = 0;
};

}

#endif // MPL_I_DATA_OBJECT_H
