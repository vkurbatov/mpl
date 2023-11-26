#ifndef MPL_PROPERTY_VALUE_IMPL_H
#define MPL_PROPERTY_VALUE_IMPL_H

#include "core/i_property_value.h"

namespace mpl
{

template<typename T>
class property_value : public i_property_value<T>
{
    T m_value;
public:

    using u_ptr_t = std::unique_ptr<property_value<T>>;
    using s_ptr_t = std::shared_ptr<property_value<T>>;

    static typename i_property_value<T>::u_ptr_t create(const T& value = {});
    static typename i_property_value<T>::u_ptr_t create(T&& value);
    static typename i_property::u_ptr_t create(const std::vector<T>& value);
    static typename i_property::s_array_t create_array(const std::vector<T>& value);

    property_value(T&& value);
    property_value(const T& value = {});

    // i_property interface
public:
    property_type_t property_type() const override;
    std::size_t size() const override;
    i_property::u_ptr_t clone() const override;
    bool merge(const i_property& property) override;
    bool is_equal(const i_property& property) const override;

    // i_property_value interface
public:
    T& get_value() override;
    const T& get_value() const override;
    void set_value(const T &value) override;
    void set_value(T&& value) override;
};


}

#endif // MPL_PROPERTY_VALUE_IMPL_H
