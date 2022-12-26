#ifndef MPL_PROPERTY_READER_H
#define MPL_PROPERTY_READER_H

#include "property_helper.h"

namespace mpl
{

class property_reader : public property_helper
{

const i_property&   m_property;

public:

    property_reader(const i_property& property);

    bool get(const std::string& key
                      , i_property::s_ptr_t& value) const;

    i_property::s_ptr_t get(const std::string& key) const;

    template<class T>
    bool get(const std::string& key
             , T& value) const
    {
        if (key.empty())
        {
            return property_helper::deserialize(value
                                                , m_property);
        }
        else if (m_property.property_type() == property_type_t::object)
        {
            if (auto property = static_cast<const i_property_tree&>(m_property).property(key))
            {
                return property_helper::deserialize(value
                                                    , *property);
            }
        }

        return false;
    }

    template<class T>
    T get(const std::string& key
          , const T& default_value) const
    {
        auto value = default_value;
        get(key, value);
        return value;
    }

    template<class T>
    std::optional<T> get(const std::string& key) const
    {
        T value = {};
        if (get(key, value))
        {
            return value;
        }
        return std::nullopt;
    }

    template<class T>
    bool check(const std::string& key, const T& value) const
    {
        T v = {};
        if (get(key, v))
        {
            return v == value;
        }
        return false;
    }

    std::vector<std::string> property_list(const std::string& key = {}
                                           , bool recursion = false) const;
};

}

#endif // MPL_PROPERTY_READER_H
