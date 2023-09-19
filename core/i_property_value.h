#ifndef MPL_I_PROPERTY_VALUE_H
#define MPL_I_PROPERTY_VALUE_H

#include "i_property.h"

namespace mpl
{

template<typename T>
class i_property_value : public i_property
{
public:
    using value_type_t = T;
    using property_type_t = i_property_value<value_type_t>;
    using s_ptr_t = std::shared_ptr<property_type_t>;
    using s_array_t = std::vector<s_ptr_t>;

    virtual ~i_property_value(){}

    virtual T& get_value() = 0;
    virtual const T& get_value() const = 0;
    virtual void set_value(const T& value) = 0;
    virtual void set_value(T&& value) = 0;
};

using i_property_array = i_property_value<i_property::s_array_t>;

}

#endif // MPL_I_PROPERTY_VALUE_H
