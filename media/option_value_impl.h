#ifndef MPL_OPTION_VALUE_IMPL_H
#define MPL_OPTION_VALUE_IMPL_H

#include "i_option_value.h"

namespace mpl
{

template<typename T>
class option_value_impl : public i_option_canonical_value<T>
{
    T                       m_value;
public:
    using canonical_type_t = option_value_impl<T>;
    using u_ptr_t = std::unique_ptr<canonical_type_t>;
    using s_ptr_t = std::shared_ptr<canonical_type_t>;

    static u_ptr_t create(const T& value)
    {
        return std::make_unique<canonical_type_t>(value);
    }

    static u_ptr_t create(T&& value = {})
    {
        return std::make_unique<canonical_type_t>(std::move(value));
    }

    option_value_impl(const T& value)
        : m_value(value)
    {

    }
    option_value_impl(T&& value = {})
        : m_value(std::move(value))
    {

    }

    // i_option_value interface
public:
    const type_info_t &type_info() const override
    {
        return type_info_t::get_type_info<T>();
    }

    bool merge(const i_option_value &option) override
    {
        if (option.type_info().type_id == type_info().type_id)
        {
            m_value = static_cast<const i_option_canonical_value<T>&>(option).get();
            return true;
        }

        return false;
    }
    bool is_equal(const i_option_value &option) const override
    {
        return option.type_info().type_id == type_info().type_id
                && m_value == static_cast<const i_option_canonical_value<T>&>(option).get();
    }

    i_option_value::u_ptr_t clone() const override
    {
        return create(m_value);
    }

    // i_option_canonical_value interface
public:
    T& get() override
    {
        return m_value;
    }

    const T& get() const override
    {
        return m_value;
    }

    void set(const T &value) override
    {
        m_value = value;
    }

    void set(T&& value) override
    {
        m_value = std::move(value);
    }
};

}

#endif // MPL_OPTION_VALUE_IMPL_H
