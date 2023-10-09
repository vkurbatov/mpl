#include "property_helper.h"
#include "common_utils.h"
#include "property_utils.h"
#include "core/common_types.h"
#include "property_value_impl.h"
#include "property_tree_impl.h"


namespace mpl
{

i_property::u_ptr_t property_helper::create_object()
{
    return utils::property::create_property(property_type_t::object);
}

i_property::u_ptr_t property_helper::create_array()
{
    return utils::property::create_property(property_type_t::array);
}

i_property::u_ptr_t property_helper::create_array(i_property::s_array_t &&values)
{
    return property_value<i_property::s_array_t>::create(std::move(values));

}

property_helper::property_helper(const i_property &property)
    : m_property(property)
{

}

const i_property* property_helper::operator[](const std::string &key) const
{
    if (key.empty())
    {
        return &m_property;
    }
    else
    {
        auto args = utils::split_lines(key
                                       , '.');

        auto p = &m_property;
        for (const auto& k : args)
        {
            if (p == nullptr
                    || p->property_type() != property_type_t::object)
            {
                p = nullptr;
                break;
            }

            p = static_cast<const i_property_tree*>(p)->property(k);
        }

        return p;
    }

    return nullptr;
}

property_type_t property_helper::get_type(const std::string &key) const
{
    if (auto p = operator [](key))
    {
        return p->property_type();
    }

    return property_type_t::undefined;
}

bool property_helper::has_property(const std::string &key) const
{
    return operator [](key) != nullptr;
}

std::vector<std::string> property_helper::property_list(const std::string &key, bool recursion) const
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
