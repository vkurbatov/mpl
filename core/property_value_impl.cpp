#include "property_value_impl.h"
#include "common_types.h"
#include "convert_utils.h"
#include "utils.h"

#include <string>
#include <cstdint>

namespace mpl
{

namespace detail
{

template<typename T>
bool compare(const T& value1
             , const T& value2)
{
    return value1 == value2;
}

template<>
bool compare(const i_property::s_array_t& value1
             , const i_property::s_array_t& value2)
{
    if (value1 == value2)
    {
        return true;
    }

    if (value1.size() == value2.size())
    {
        auto it1 = value1.begin();
        auto it2 = value2.begin();

        while(it1 != value1.end())
        {

            if (*it1 == *it2)
            {
                continue;
            }

            if (*it1 == nullptr
                    || *it2 == nullptr
                    || !(*it1)->is_equal(**it2))
            {
                return false;
            }

            it1++;
            it2++;
        }

        return true;
    }

    return false;
}

}

// template declarations
template class property_value<std::int8_t>;
template class property_value<std::int16_t>;
template class property_value<std::int32_t>;
template class property_value<std::int64_t>;
template class property_value<std::uint8_t>;
template class property_value<std::uint16_t>;
template class property_value<std::uint32_t>;
template class property_value<std::uint64_t>;

template class property_value<bool>;

template class property_value<float>;
template class property_value<double>;
template class property_value<long double>;

template class property_value<std::string>;
template class property_value<octet_string_t>;

template class property_value<i_property::s_array_t>;

template<typename T>
typename i_property_value<T>::u_ptr_t property_value<T>::create(const T &value)
{
    return std::make_unique<property_value>(value);
}

template<typename T>
typename i_property_value<T>::u_ptr_t property_value<T>::create(T &&value)
{
    return std::make_unique<property_value>(std::move(value));
}

template<typename T>
typename i_property::u_ptr_t property_value<T>::create(const std::vector<T> &value)
{
    return property_value<i_property::s_array_t>::create(property_value<T>::create_array(value));
}

template<typename T>
typename i_property::s_array_t property_value<T>::create_array(const std::vector<T> &value)
{
    i_property::s_array_t array;

    for (const auto& v : value)
    {
        array.emplace_back(property_value::create(v));
    }

    return array;
}

template<typename T>
property_value<T>::property_value(T &&value)
    : m_value(std::move(value))
{

}

template<typename T>
property_value<T>::property_value(const T &value)
    : m_value(value)
{

}

template<typename T>
property_type_t property_value<T>::property_type() const
{
    return core::utils::get_property_type<T>();
}

template<typename T>
std::size_t property_value<T>::size() const
{
    return core::utils::get_value_size(m_value);
}

template<typename T>
i_property::u_ptr_t property_value<T>::clone() const
{
    return create(m_value);
}

template<typename T>
bool property_value<T>::merge(const i_property &property)
{
    return core::utils::convert(property
                                , m_value);
}

template<typename T>
bool property_value<T>::is_equal(const i_property &property) const
{
    if (property_type() == property.property_type())
    {
        return detail::compare(m_value
                               , static_cast<const i_property_value<T>&>(property).get_value());
    }

    return false;
}

template<typename T>
T &property_value<T>::get_value()
{
    return m_value;
}

template<typename T>
const T &property_value<T>::get_value() const
{
    return m_value;
}

template<typename T>
void property_value<T>::set_value(const T &value)
{
    m_value = value;
}

template<typename T>
void property_value<T>::set_value(T &&value)
{
    m_value = std::move(value);
}

}
