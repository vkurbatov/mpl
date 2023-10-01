#include "property_helper.h"
#include "common_utils.h"
#include "core/common_types.h"
#include "property_value_impl.h"
#include "property_tree_impl.h"


namespace mpl
{

#define __declare_base_serializer_type(type)\
    template bool property_helper::serialize<type>(const type& value, i_property& property);\
    template<> i_property::u_ptr_t property_helper::serialize(const type& value) { return property_value<type>::create(value); }\
    template bool property_helper::deserialize<type>(type& value, const i_property &property);

#define __declare_serializer_type(type)\
    __declare_base_serializer_type(type)

__declare_serializer_type(std::int8_t)
__declare_serializer_type(std::int16_t)
__declare_serializer_type(std::int32_t)
__declare_serializer_type(std::int64_t)

__declare_serializer_type(std::uint8_t)
__declare_serializer_type(std::uint16_t)
__declare_serializer_type(std::uint32_t)
__declare_serializer_type(std::uint64_t)

__declare_serializer_type(bool)

__declare_serializer_type(float)
__declare_serializer_type(double)
__declare_serializer_type(long double)

__declare_serializer_type(std::string)
__declare_serializer_type(octet_string_t)

i_property::u_ptr_t property_helper::create_property(property_type_t type)
{
    switch(type)
    {
        case property_type_t::object:
            return property_tree::create();
        break;
        case property_type_t::array:
            return property_value<i_property::s_array_t>::create();
        break;
        case property_type_t::i8:
            return property_value<std::int8_t>::create();
        break;
        case property_type_t::i16:
            return property_value<std::int16_t>::create();
        break;
        case property_type_t::i32:
            return property_value<std::int32_t>::create();
        break;
        case property_type_t::i64:
            return property_value<std::int64_t>::create();
        break;
        case property_type_t::u8:
            return property_value<std::uint8_t>::create();
        break;
        case property_type_t::u16:
            return property_value<std::uint16_t>::create();
        break;
        case property_type_t::u32:
            return property_value<std::uint32_t>::create();
        break;
        case property_type_t::u64:
            return property_value<std::uint64_t>::create();
        break;
        case property_type_t::r32:
            return property_value<float>::create();
        break;
        case property_type_t::r64:
            return property_value<double>::create();
        break;
        case property_type_t::r96:
            return property_value<long double>::create();
        break;
        case property_type_t::boolean:
            return property_value<bool>::create();
        break;
        case property_type_t::string:
            return property_value<std::string>::create();
        break;
        case property_type_t::octet_string:
            return property_value<octet_string_t>::create();
        break;
        default:;
    }

    return nullptr;
}

i_property::u_ptr_t property_helper::create_object()
{
    return property_tree::create();
}

i_property::u_ptr_t property_helper::create_array(i_property::s_array_t&& values)
{
    return property_value<i_property::s_array_t>::create(std::move(values));
}

template<class T>
bool property_helper::deserialize(T &value, const i_property &property)
{
    return utils::convert(property, value);
}

template<>
bool property_helper::deserialize(octet_string_t& value
                                        , const i_property &property)
{
     return utils::convert(property, value);
}

bool property_helper::deserialize(i_property::s_array_t &value
                                 , const i_property &property)
{
    if (property.property_type() == property_type_t::array)
    {
        for (auto&& p : static_cast<const i_property_value<i_property::s_array_t>&>(property).get_value())
        {
            if (p != nullptr)
            {
                value.emplace_back(p->clone());
            }
        }
        return true;
    }
    return false;
}


template<class T>
bool property_helper::serialize(const T &value
                                , i_property& property)
{
    return utils::convert(value, property);
}

bool property_helper::serialize(const i_property::s_array_t &value
                                , i_property &property)
{
    if (property.property_type() == property_type_t::array)
    {
        auto& array_value = static_cast<i_property_array&>(property);
        array_value.set_value(value);
        return true;
    }

    return false;
}

i_property::u_ptr_t property_helper::serialize(const i_property::s_array_t &value)
{
    return property_value<i_property::s_array_t>::create(value);
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
