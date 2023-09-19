#ifndef MPL_I_PROPERTY_H
#define MPL_I_PROPERTY_H

#include "property_types.h"

#include <memory>
#include <vector>

namespace mpl
{

class i_property
{
public:
    using u_ptr_t = std::unique_ptr<i_property>;
    using s_ptr_t = std::shared_ptr<i_property>;
    using const_u_ptr_t = std::unique_ptr<const i_property>;
    using const_s_ptr_t = std::shared_ptr<const i_property>;
    using s_array_t = std::vector<s_ptr_t>;

    virtual ~i_property() {}

    virtual property_type_t property_type() const = 0;
    virtual std::size_t size() const = 0; // size value or count
    virtual u_ptr_t clone() const = 0;
    virtual bool merge(const i_property& property) = 0;
    virtual bool is_equal(const i_property& property) const = 0;
};

}

#endif // MPL_I_PROPERTY_H
