#include "property_writer.h"

namespace mpl
{

property_writer::property_writer(i_property &property)
    : property_reader(property)
    , m_property(property)
{

}

i_property* property_writer::create_tree(const std::string &key, bool create_always)
{
    if (!key.empty()
            && m_property.property_type() == property_type_t::object)
    {
        auto& tree = static_cast<i_property_tree&>(m_property);

        if (!create_always)
        {
            if (auto p = tree.property(key))
            {
                if (p->property_type() == property_type_t::object)
                {
                    return p;
                }
            }
        }

        if (auto object = property_helper::create_tree())
        {
            tree.set(key
                              , std::move(object));

            return tree.property(key);
        }
    }

    return nullptr;
}

i_property* property_writer::create_array(const std::string &key, bool create_always)
{
    if (!key.empty()
            && m_property.property_type() == property_type_t::object)
    {
        auto& tree = static_cast<i_property_tree&>(m_property);

        if (!create_always)
        {
            if (auto p = tree.property(key))
            {
                if (p->property_type() == property_type_t::array)
                {
                    return p;
                }
            }
        }

        if (auto object = property_helper::create_array())
        {
            tree.set(key
                     , std::move(object));

            return tree.property(key);
        }
    }

    return nullptr;
}

i_property* property_writer::operator[](const std::string &key)
{
    return const_cast<i_property*>(property_helper::operator [](key));
}

bool property_writer::set(const std::string &key, const i_property &property)
{
    if (auto p = operator [](key))
    {
        return p->merge(property);
    }

    return false;
}

bool property_writer::set(const std::string &key
                          , const i_property *property)
{
    return property != nullptr
            && set(key, *property);
}

bool property_writer::remove(const std::string &key)
{
    if (!key.empty()
            && m_property.property_type() == property_type_t::object)
    {
        static_cast<i_property_tree&>(m_property).set(key
                                                               , nullptr);

        return true;
    }
    return false;
}

bool property_writer::merge(const std::string &key
                           , const i_property &property)
{
    auto object = operator [](key);
    if (!object)
    {
        object = create_tree(key);
    }

    return object != nullptr
            && object->merge(property);

}

bool property_writer::merge(const std::string &dst_key
                           , const std::string &src_key
                           , const i_property &property)
{
    if (auto p = property_reader(property)[src_key])
    {
        return merge(dst_key
                     , *p);
    }

    return false;
}

}
