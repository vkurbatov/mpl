#include "convert_utils.h"

#include "common_types.h"
#include "i_property_value.h"

#include <string>

namespace mpl::core::utils
{

namespace detail
{

bool convert(const i_property::s_array_t& in_value, i_property::s_array_t& out_value)
{
    out_value.clear();
    for (const auto& i : in_value)
    {
        if (i != nullptr)
        {
            out_value.emplace_back(i->clone());
        }
    }
    return true;
}

template<typename Tin>
bool convert(const Tin& in_value, i_property::s_array_t& out_value)
{
    return false;
}

template<typename Tout>
bool convert(const i_property::s_array_t& out_value, Tout& in_value)
{
    return false;
}

bool convert(const std::string& in_value, i_property::s_array_t &out_value)
{
    return false;
}

bool convert(const octet_string_t& in_value, i_property::s_array_t &out_value)
{
    return false;
}

bool convert(const i_property::s_array_t &in_value, std::string& out_value)
{
    return false;
}

template<typename Tin, typename Tout>
bool convert(const Tin& in_value, Tout& out_value)
{
    return mpl::core::utils::convert(in_value
                                      , out_value);
}

template<typename Tin, typename Tout>
bool convert_property(const Tin& in_value, i_property_value<Tout>& out_value)
{
    Tout value = {};
    if (convert(in_value, value))
    {
        out_value.set_value(value);
        return true;
    }

    return false;
}

template<typename Tout>
bool convert_from_property(const i_property &in_value, Tout &out_value)
{
    switch(in_value.property_type())
    {
        case property_type_t::i8:
            return detail::convert(static_cast<const i_property_value<std::int8_t>&>(in_value).get_value(), out_value);
        break;
        case property_type_t::i16:
            return detail::convert(static_cast<const i_property_value<std::int16_t>&>(in_value).get_value(), out_value);
        break;
        case property_type_t::i32:
            return detail::convert(static_cast<const i_property_value<std::int32_t>&>(in_value).get_value(), out_value);
        break;
        case property_type_t::i64:
            return detail::convert(static_cast<const i_property_value<std::int64_t>&>(in_value).get_value(), out_value);
        break;
        case property_type_t::u8:
            return detail::convert(static_cast<const i_property_value<std::uint8_t>&>(in_value).get_value(), out_value);
        break;
        case property_type_t::u16:
            return detail::convert(static_cast<const i_property_value<std::uint16_t>&>(in_value).get_value(), out_value);
        break;
        case property_type_t::u32:
            return detail::convert(static_cast<const i_property_value<std::uint32_t>&>(in_value).get_value(), out_value);
        break;
        case property_type_t::u64:
            return detail::convert(static_cast<const i_property_value<std::uint64_t>&>(in_value).get_value(), out_value);
        break;
        case property_type_t::boolean:
            return detail::convert(static_cast<const i_property_value<bool>&>(in_value).get_value(), out_value);
        break;
        case property_type_t::r32:
            return detail::convert(static_cast<const i_property_value<float>&>(in_value).get_value(), out_value);
        break;
        case property_type_t::r64:
            return detail::convert(static_cast<const i_property_value<double>&>(in_value).get_value(), out_value);
        break;
        case property_type_t::r96:
            return detail::convert(static_cast<const i_property_value<long double>&>(in_value).get_value(), out_value);
        break;
        case property_type_t::string:
            return detail::convert(static_cast<const i_property_value<std::string>&>(in_value).get_value(), out_value);
        break;
        case property_type_t::octet_string:
            return detail::convert(static_cast<const i_property_value<octet_string_t>&>(in_value).get_value(), out_value);
        break;
        case property_type_t::array:
            return detail::convert(static_cast<const i_property_value<i_property::s_array_t>&>(in_value).get_value(), out_value);
        break;
        default:
            return false;
        break;

    }

    return false;
}

template<typename Tin>
bool convert_to_property(const Tin &in_value, i_property &out_value)
{
    switch(out_value.property_type())
    {
        case property_type_t::i8:
            return detail::convert_property(in_value, static_cast<i_property_value<std::int8_t>&>(out_value));
        break;
        case property_type_t::i16:
            return detail::convert_property(in_value, static_cast<i_property_value<std::int16_t>&>(out_value));
        break;
        case property_type_t::i32:
            return detail::convert_property(in_value, static_cast<i_property_value<std::int32_t>&>(out_value));
        break;
        case property_type_t::i64:
            return detail::convert_property(in_value, static_cast<i_property_value<std::int64_t>&>(out_value));
        break;
        case property_type_t::u8:
            return detail::convert_property(in_value, static_cast<i_property_value<std::uint8_t>&>(out_value));
        break;
        case property_type_t::u16:
            return detail::convert_property(in_value, static_cast<i_property_value<std::uint16_t>&>(out_value));
        break;
        case property_type_t::u32:
            return detail::convert_property(in_value, static_cast<i_property_value<std::uint32_t>&>(out_value));
        break;
        case property_type_t::u64:
            return detail::convert_property(in_value, static_cast<i_property_value<std::uint64_t>&>(out_value));
        break;
        case property_type_t::boolean:
            return detail::convert_property(in_value, static_cast<i_property_value<bool>&>(out_value));
        break;
        case property_type_t::r32:
            return detail::convert_property(in_value, static_cast<i_property_value<float>&>(out_value));
        break;
        case property_type_t::r64:
            return detail::convert_property(in_value, static_cast<i_property_value<double>&>(out_value));
        break;
        case property_type_t::r96:
            return detail::convert_property(in_value, static_cast<i_property_value<long double>&>(out_value));
        break;
        case property_type_t::string:
            return detail::convert_property(in_value, static_cast<i_property_value<std::string>&>(out_value));
        break;
        case property_type_t::octet_string:
            return detail::convert_property(in_value, static_cast<i_property_value<octet_string_t>&>(out_value));
        break;
        case property_type_t::array:
            return detail::convert_property(in_value, static_cast<i_property_value<i_property::s_array_t>&>(out_value));
        break;
        default:
            return false;
        break;
    }

    return false;
}

}

#define __declare_conversion_type(type)\
    template<> bool convert(const i_property &in_value, type& out_value) { return detail::convert_from_property(in_value, out_value); }\
    template<> bool convert(const type &in_value, i_property& out_value) { return detail::convert_to_property(in_value, out_value); }

__declare_conversion_type(std::int8_t)
__declare_conversion_type(std::int16_t)
__declare_conversion_type(std::int32_t)
__declare_conversion_type(std::int64_t)

__declare_conversion_type(std::uint8_t)
__declare_conversion_type(std::uint16_t)
__declare_conversion_type(std::uint32_t)
__declare_conversion_type(std::uint64_t)

__declare_conversion_type(bool)

__declare_conversion_type(float)
__declare_conversion_type(double)
__declare_conversion_type(long double)

__declare_conversion_type(std::string)
__declare_conversion_type(octet_string_t)
__declare_conversion_type(i_property::s_array_t)

}
