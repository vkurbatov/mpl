#include "property_reader.h"

namespace mpl
{

property_reader::property_reader(const i_property &property)
    : property_helper(property)
    , m_property(property)
{

}

bool property_reader::get(const std::string &key, i_property::s_ptr_t &value) const
{
    if (auto p = operator [](key))
    {
        value = p->clone();
        return value != nullptr;
    }

    return false;
}

i_property::s_ptr_t property_reader::get(const std::string &key) const
{
    if (auto p = operator [](key))
    {
        return p->clone();
    }

    return nullptr;
}

std::vector<std::string> property_reader::property_list(const std::string &key
                                                              , bool recursion) const
{
    if (auto object = operator [](key))
    {
        if (object->property_type() == property_type_t::object)
        {
            return static_cast<const i_property_tree&>(*object).property_list(recursion);
        }
    }

    return {};
}


}
