#ifndef MPL_PROPERTY_UTILS_H
#define MPL_PROPERTY_UTILS_H

#include "core/i_property_tree.h"
#include "core/i_property_value.h"
#include "convert_utils.h"

#include <cstdint>
#include <string>
#include <set>
#include <map>
#include <optional>

namespace mpl::utils::property
{

template<class ...Args>
std::string build_key(const Args&... args)
{
    std::string key;
    std::string string_args[] = { args... };
    for (const auto& a: string_args)
    {

        if (!a.empty())
        {
            if (!key.empty())
            {
                key.append(".");
            }
            key.append(a);
        }
    }

    return key;
}

i_property::u_ptr_t create_property(property_type_t type);

template<typename T>
bool deserialize(T& value, const i_property& property);

bool deserialize(i_property::s_array_t& value, const i_property& property);

// vector
template<class T>
bool deserialize(std::vector<T>& value, const i_property& property)
{
    bool result = false;
    if (property.property_type() == property_type_t::array)
    {
        const auto& array = static_cast<const i_property_array&>(property).get_value();
        value.resize(array.size());
        result = value.empty();
        for (std::size_t idx = 0; idx < array.size(); idx++)
        {
            if (array[idx] != nullptr)
            {
                if (deserialize(value[idx]
                                , *array[idx]))
                {
                    result = true;
                }
            }
        }
    }
    return result;
}

// set
template<class T>
bool deserialize(std::set<T>& value, const i_property& property)
{
    if (property.property_type() == property_type_t::array)
    {
        bool result = true;
        for (const auto& p : static_cast<const i_property_array&>(property).get_value())
        {
            T element = {};
            if (p != nullptr
                    && deserialize(element, *p))
            {
                value.emplace(std::move(element));
                continue;
            }
            result = false;
        }
        return result;
    }
    return false;
}

// map
template<class K, class T>
bool deserialize(std::map<K,T>& value, const i_property& property)
{
    if (property.property_type() == property_type_t::object)
    {
        const auto& tree = static_cast<const i_property_tree&>(property);
        bool result = true;

        for (const auto& k : tree.property_list())
        {
            if (auto p = tree.property(k))
            {
                std::pair<K,T> element = {};
                if (utils::convert(k, element.first)
                        && deserialize(element.second
                                       , *p))
                {
                    value.emplace(element);
                    continue;
                }
            }
            result = false;
        }
        return result;
    }

    return false;
}

template<typename T>
bool serialize(const T& value, const i_property& property);

bool serialize(const i_property::s_array_t& value, i_property& property);
i_property::u_ptr_t serialize(const i_property::s_array_t& value);

template<class T>
bool serialize(const T& value, i_property& property);

template<class T>
i_property::u_ptr_t serialize(const T& value)
{
    if (auto object = create_property(property_type_t::object))
    {
        if (serialize(value
                      , *object))
        {
            return object;
        }
    }

    return nullptr;
}

// optional
template<class T>
bool serialize(const std::optional<T>& value, i_property& property)
{
    if (value)
    {
        return serialize(*value
                         , property);
    }

    return false;
}

template<class T>
i_property::u_ptr_t serialize(const std::optional<T>& value)
{
    if (value)
    {
        return serialize(*value);
    }

    return nullptr;
}

// vector
template<class T>
bool serialize(const std::vector<T>& value, i_property& property)
{
    if (property.property_type() == property_type_t::array)
    {
        i_property::s_array_t array;
        for (const auto& v : value)
        {
            array.emplace_back(std::move(serialize(v)));
        }

        static_cast<i_property_array&>(property).set_value(std::move(array));
        return true;
    }

    return false;
}

template<class T>
i_property::u_ptr_t serialize(const std::vector<T>& value)
{
    i_property::s_array_t array;
    for (const auto& v : value)
    {
        array.emplace_back(std::move(serialize(v)));
    }

    return serialize(array);
}

// pair
template<class K, class T>
bool serialize(const std::pair<K,T>& value, i_property& property)
{
    if (property.property_type() == property_type_t::object)
    {
        std::string key;
        if (utils::convert(value.first
                           , key))
        {
            static_cast<i_property_tree&>(property).set(key
                                                        , serialize(value.second));
            return true;
        }
    }

    return false;
}

// set
template<class T>
bool serialize(const std::set<T>& value, i_property& property)
{
    if (property.property_type() == property_type_t::array)
    {
        i_property::s_array_t array;
        for (const auto& v : value)
        {
            array.emplace_back(std::move(serialize(v)));
        }

        static_cast<i_property_array&>(property).set_value(std::move(array));
        return true;
    }

    return false;
}

template<class T>
i_property::u_ptr_t serialize(const std::set<T>& value)
{
    i_property::s_array_t array;
    for (const auto& v : value)
    {
        array.emplace_back(std::move(serialize(v)));
    }

    return serialize(array);
}

// map
template<class K, class T>
bool serialize(const std::map<K,T>& value
                      , i_property& property)
{
    if (property.property_type() == property_type_t::object)
    {
        for (const auto& v : value)
        {
            serialize(v, property);
        }

        return true;
    }

    return false;
}

template<class K, class T>
i_property::u_ptr_t serialize(const std::map<K,T>& value)
{
    if (auto object = create_property(property_type_t::object))
    {
        for (const auto& v : value)
        {
            serialize(v
                      , *object);
        }
        return object;
    }

    return nullptr;
}

template<typename T>
property_type_t get_property_type();

property_class_t get_property_class(property_type_t type);

template<typename T>
std::size_t get_value_size(const T& value);


}

#endif // MPL_PROPERTY_UTILS_H
