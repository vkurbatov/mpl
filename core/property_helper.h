#ifndef MPL_PROPERTY_HELPER_H
#define MPL_PROPERTY_HELPER_H

#include "i_property_value.h"
#include "i_property_tree.h"

#include "convert_utils.h"

#include <string>
#include <set>
#include <map>
#include <optional>

namespace mpl
{

class property_helper
{
    const i_property&   m_property;
public:
    static i_property::u_ptr_t create_property(property_type_t type);
    static i_property::u_ptr_t create_object();
    static i_property::u_ptr_t create_array(i_property::array_t&& values = {});

    template<class T>
    static bool deserialize(T& value, const i_property& property);

    static bool deserialize(i_property::array_t& value, const i_property& property);

    // vector
    template<class T>
    static bool deserialize(std::vector<T>& value, const i_property& property)
    {
        bool result = true;
        if (property.property_type() == property_type_t::array)
        {
            for (const auto& p : static_cast<const i_property_array&>(property).get_value())
            {
                T element = {};
                if (p != nullptr
                        && deserialize(element, *p))
                {
                    value.emplace_back(std::move(element));
                    continue;
                }
                result = false;
            }
            return result;
        }
        return false;
    }

    // set
    template<class T>
    static bool deserialize(std::set<T>& value, const i_property& property)
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
    static bool deserialize(std::map<K,T>& value, const i_property& property)
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
                    if (core::utils::convert(k, element.first)
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

    static bool serialize(const i_property::array_t& value, i_property& property);
    static i_property::u_ptr_t serialize(const i_property::array_t& value);

    template<class T>
    static bool serialize(const T& value, i_property& property);

    template<class T>
    static i_property::u_ptr_t serialize(const T& value)
    {
        if (auto object = create_object())
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
    static bool serialize(const std::optional<T>& value, i_property& property)
    {
        if (value)
        {
            return serialize(*value
                             , property);
        }

        return false;
    }

    template<class T>
    static i_property::u_ptr_t serialize(const std::optional<T>& value)
    {
        if (value)
        {
            return serialize(*value);
        }

        return nullptr;
    }

    // vector
    template<class T>
    static bool serialize(const std::vector<T>& value, i_property& property)
    {
        if (property.property_type() == property_type_t::array)
        {
            i_property::array_t array;
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
    static i_property::u_ptr_t serialize(const std::vector<T>& value)
    {
        i_property::array_t array;
        for (const auto& v : value)
        {
            array.emplace_back(std::move(serialize(v)));
        }

        return property_helper::serialize(array);
    }

    // pair
    template<class K, class T>
    static bool serialize(const std::pair<K,T>& value, i_property& property)
    {
        if (property.property_type() == property_type_t::object)
        {
            std::string key;
            if (core::utils::convert(value.first
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
    static bool serialize(const std::set<T>& value, i_property& property)
    {
        if (property.property_type() == property_type_t::array)
        {
            i_property::array_t array;
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
    static i_property::u_ptr_t serialize(const std::set<T>& value)
    {
        i_property::array_t array;
        for (const auto& v : value)
        {
            array.emplace_back(std::move(serialize(v)));
        }

        return serialize(array);
    }

    // map
    template<class K, class T>
    static bool serialize(const std::map<K,T>& value
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
    static i_property::u_ptr_t serialize(const std::map<K,T>& value)
    {
        if (auto object = create_object())
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

    property_helper(const i_property& property);

    const i_property* operator[](const std::string& key) const;
    property_type_t get_type(const std::string& key) const;
    bool has_property(const std::string& key) const;
    std::vector<std::string> property_list(const std::string& key = {}
                                           , bool recursion = false) const;
};

}

#endif // MPL_PROPERTY_HELPER_H
