#ifndef MPL_PROPERTY_WRITER_H
#define MPL_PROPERTY_WRITER_H

#include "property_reader.h"

namespace mpl
{

class property_writer : public property_reader
{
    i_property&     m_property;

public:

    property_writer(i_property& property);

    i_property* create_tree(const std::string& key, bool create_always = true);
    i_property* create_array(const std::string& key, bool create_always = true);
    i_property* operator[](const std::string& key);

    bool set(const std::string& key, const i_property& property);
    bool set(const std::string& key, const i_property* property);

    template<class T>
    bool set(const std::string& key
                      , const T& value)
    {
        if (key.empty())
        {
            return serialize(value, m_property);
        }
        else if (m_property.property_type() == property_type_t::object)
        {
            static_cast<i_property_tree&>(m_property).set(key
                                                         , property_helper::serialize(value));
            return true;
        }

        return false;
    }

    template<class T>
    bool set(const std::string& key
                      , const T& value
                      , const T& default_value)
    {
        if (value != default_value)
        {
            return set(key
                       , value);
        }
        else
        {
            return remove(key);
        }

        return true;
    }

    template<class T>
    bool append(const std::string& key
                , const T& value)
    {
        if (auto node = operator [](key))
        {
            if (node->property_type() == property_type_t::array)
            {
                auto& array = static_cast<i_property_array&>(*node).get_value();
                if (auto p = property_helper::serialize(value))
                {
                    array.emplace_back(std::move(p));
                    return true;
                }
            }
            //return property_serializer(*node).append_value<T>(value);
        }
        else
        {
            return set<std::vector<T>>(key
                                       , { value }
                                       );
        }

        return false;
    }

    bool remove(const std::string& key);
    bool merge(const std::string& key, const i_property& property);
    bool merge(const std::string& dst_key
               , const std::string& src_key
               , const i_property& property);
};

}

#endif // MPL_PROPERTY_WRITER_H
