#include "property_utils.h"
#include "property_tree_impl.h"
#include "property_value_impl.h"

#include "common_utils.h"

namespace mpl::utils::property
{

#define __declare_value_size(type) template std::size_t get_value_size(const type& value);

#define __declare_property_value_type(type, ptype) template<> property_type_t get_property_type<type>() { return property_type_t::ptype; }
#define __declare_property_array_type(type, ptype)\
    __declare_property_value_type(std::vector<type>, array)

#define __declare_property_type(type, ptype, pclass)\
    __declare_property_value_type(type, ptype)\
    __declare_property_array_type(type, ptype)

#define __declare_all_property_types(type, ptype, pclass)\
    __declare_property_type(type, ptype, pclass)\
    __declare_value_size(type)\
    __declare_value_size(std::vector<type>)


__declare_all_property_types(std::int8_t, i8, s_numeric)
__declare_all_property_types(std::int16_t, i16, s_numeric)
__declare_all_property_types(std::int32_t, i32, s_numeric)
__declare_all_property_types(std::int64_t, i64, s_numeric)

__declare_property_value_type(std::uint8_t, u8)
__declare_value_size(std::uint8_t)

__declare_all_property_types(std::uint16_t, u16, u_numeric)
__declare_all_property_types(std::uint32_t, u32, u_numeric)
__declare_all_property_types(std::uint64_t, u64, u_numeric)
__declare_all_property_types(bool, boolean, boolean)

__declare_all_property_types(float, r32, real)
__declare_all_property_types(double, r64, real)
__declare_all_property_types(long double, r96, real)

__declare_property_type(std::string, string, string)
__declare_value_size(std::vector<std::string>)
//__declare_property_type(char*, string)

__declare_all_property_types(octet_string_t, octet_string, octet_string)

template<> property_type_t get_property_type<std::vector<i_property::s_ptr_t>>() { return property_type_t::array; }
template std::size_t get_value_size(const std::vector<i_property::s_ptr_t>& value);

template<>
std::size_t get_value_size(const std::string &value)
{
    return value.size();
}

template<typename T>
std::size_t get_value_size(const std::vector<T> &value)
{
    return value.size();
}

template<typename T>
std::size_t get_value_size(const T &value)
{
    return sizeof(value);
}

property_class_t get_property_class(property_type_t type)
{
    switch(type)
    {
        case property_type_t::object:
            return property_class_t::object;
        break;
        case property_type_t::array:
            return property_class_t::array;
        break;
        case property_type_t::i8:
        case property_type_t::i16:
        case property_type_t::i32:
        case property_type_t::i64:
            return property_class_t::s_numeric;
        break;
        case property_type_t::u8:
        case property_type_t::u16:
        case property_type_t::u32:
        case property_type_t::u64:
            return property_class_t::u_numeric;
        break;
        case property_type_t::r32:
        case property_type_t::r64:
        case property_type_t::r96:
            return property_class_t::real;
        break;
        case property_type_t::boolean:
            return property_class_t::boolean;
        break;
        case property_type_t::string:
            return property_class_t::string;
        break;
        case property_type_t::octet_string:
            return property_class_t::octet_string;
        break;
    }

    return property_class_t::undefined;
}

#define __declare_base_serializer_type(type)\
    template bool serialize<type>(const type& value, i_property& property);\
    template<> i_property::u_ptr_t serialize(const type& value) { return property_value<type>::create(value); }\
    template bool deserialize<type>(type& value, const i_property &property);

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

template<class T>
bool deserialize(T &value, const i_property &property)
{
    return utils::convert(property, value);
}

template<>
bool deserialize(octet_string_t& value
                  , const i_property &property)
{
     return utils::convert(property, value);
}

bool deserialize(i_property::s_array_t &value
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
bool serialize(const T &value
              , i_property& property)
{
    return utils::convert(value, property);
}

bool serialize(const i_property::s_array_t &value
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

i_property::u_ptr_t serialize(const i_property::s_array_t &value)
{
    return property_value<i_property::s_array_t>::create(value);
}

i_property::u_ptr_t create_property(property_type_t type)
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


}
