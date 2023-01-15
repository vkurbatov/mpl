#ifndef MPL_I_OPTION_VALUE_H
#define MPL_I_OPTION_VALUE_H

#include <memory>
#include <cstdint>
#include "type_info.h"

namespace mpl
{

class i_option_value
{
public:

    using u_ptr_t = std::unique_ptr<i_option_value>;
    using s_ptr_t = std::shared_ptr<i_option_value>;

    virtual ~i_option_value() = default;
    virtual const type_info_t& type_info() const = 0;
    virtual bool merge(const i_option_value& option) = 0;
    virtual bool is_equal(const i_option_value& option) const = 0;
    virtual u_ptr_t clone() const = 0;
};

template<typename T>
class i_option_canonical_value : public i_option_value
{
public:
    using canonical_type_t = i_option_canonical_value<T>;
    using u_ptr_t = std::unique_ptr<canonical_type_t>;
    using s_ptr_t = std::shared_ptr<canonical_type_t>;

    virtual T& get() = 0;
    virtual const T& get() const = 0;
    virtual void set(const T& value) = 0;
    virtual void set(T&& value) = 0;
};

}

#endif // MPL_I_OPTION_VALUE_H
