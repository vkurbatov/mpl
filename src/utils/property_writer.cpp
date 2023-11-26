#include "property_writer.h"

namespace mpl
{

property_writer::property_writer(i_property &property)
    : property_reader(property)
    , m_property(property)
{

}

i_property* property_writer::create_object(const std::string &key, bool create_always)
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

        if (auto object = utils::property::create_property(property_type_t::object))
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

        if (auto object = utils::property::create_property(property_type_t::array))
        {
            tree.set(key
                     , std::move(object));

            return tree.property(key);
        }
    }

    return nullptr;
}

i_property *property_writer::create(const std::string &key
                                    , const i_property &property)
{
    if (!key.empty()
            && m_property.property_type() == property_type_t::object)
    {
        auto& tree = static_cast<i_property_tree&>(m_property);
        if (auto clone_value = property.clone())
        {
            auto p = clone_value.get();
            if (tree.set(key
                         , std::move(clone_value)))
            {
                return p;
            }
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
    if (key.empty())
    {
        return m_property.merge(property);
    }
    else
    {
        return create(key
                      , property) != nullptr;
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
        object = create_object(key);
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

bool property_writer::clear()
{
    switch(m_property.property_type())
    {
        case property_type_t::object:
            static_cast<i_property_tree&>(m_property).clear();
            return true;
        break;
        case property_type_t::array:
            static_cast<i_property_array&>(m_property).get_value().clear();
            return true;
        break;
        case property_type_t::string:
            static_cast<i_property_value<std::string>&>(m_property).get_value().clear();
            return true;
        break;
        case property_type_t::octet_string:
            static_cast<i_property_value<std::vector<std::uint8_t>>&>(m_property).get_value().clear();
            return true;
        break;
        default:;
    }

    return false;
}

}
