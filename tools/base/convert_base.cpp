#include "convert_base.h"

#include <vector>
#include <cstring>
#include <sstream>

namespace base
{

using octet_string_t = std::vector<std::uint8_t>;

namespace detail
{

bool convert(const std::string& in_value, std::string& out_value)
{
    out_value = in_value;
    return true;
}

bool convert(const octet_string_t& in_value, octet_string_t& out_value)
{
    out_value = in_value;
    return true;
}

bool convert(const octet_string_t& in_value, std::string &out_value)
{
    out_value.reserve(in_value.size() * 2 + 1);
    char hex[3];
    for (const auto& v : in_value)
    {
        std::sprintf(hex, "%02x", v);
        out_value.append(hex);
    }
    return true;
}

template<typename Tin, typename Tout>
bool convert(const Tin& in_value, Tout &out_value)
{
    out_value = static_cast<Tout>(in_value);
    return true;
}

template<typename Tin>
bool convert(const Tin& in_value, std::string &out_value)
{
    std::ostringstream ss;
    ss << in_value;
    out_value = ss.str();
    return true;
}

template<typename Tout>
bool convert(const std::string& in_value, Tout &out_value)
{
    std::stringstream ss;
    ss << in_value;
    if (ss >> out_value)
    {
        return true;
    }
    return false;
}

template<typename Tout>
bool convert(const octet_string_t& in_value, Tout &out_value)
{
    if (in_value.size() == sizeof(out_value))
    {
        std::memcpy(&out_value, in_value.data(), sizeof(out_value));
        return true;
    }

    return false;
}

template<typename T>
bool convert(const T& in_value, T &out_value)
{
    out_value = in_value;
    return true;
}

template<>
bool convert(const std::string& in_value, octet_string_t &out_value)
{
    try
    {
        for (auto i = 0u; i < in_value.size(); i += 2)
        {
            out_value.push_back(std::stoi(in_value.substr(i, 2), 0, 16));
        }
    }
    catch(...)
    {
        out_value.clear();
        return false;
    }

    return !out_value.empty();
}

template<typename Tin, typename Tout>
std::optional<Tout> convert(const Tin& in_value)
{
    Tout out_value = {};
    if (detail::convert(in_value, out_value))
    {
        return out_value;
    }

    return {};
}

std::size_t get_value_size(const std::string& value)
{
    return value.size();
}

std::size_t get_value_size(const octet_string_t& value)
{
    return value.size();
}

template<typename T>
std::size_t get_value_size(const T& value)
{
    return sizeof(value);
}

} // detail

#define declare_conversion(in_type, out_type)\
    template<> bool convert(const in_type &in_value, out_type& out_value) { return detail::convert(in_value, out_value); }\

#define declare_conversion_type(type, vtype)\
    template<> value_type_t get_value_type<type>() { return value_type_t::vtype; };\
    template std::size_t get_value_size(const type& value);\
    declare_conversion(type, std::int8_t)\
    declare_conversion(type, std::int16_t)\
    declare_conversion(type, std::int32_t)\
    declare_conversion(type, std::int64_t)\
    declare_conversion(type, std::uint8_t)\
    declare_conversion(type, std::uint16_t)\
    declare_conversion(type, std::uint32_t)\
    declare_conversion(type, std::uint64_t)\
    declare_conversion(type, float)\
    declare_conversion(type, double)\
    declare_conversion(type, long double)\
    declare_conversion(type, bool)\
    declare_conversion(type, std::string)\
    declare_conversion(type, octet_string_t)


declare_conversion_type(std::int8_t, i8)
declare_conversion_type(std::int16_t, i16)
declare_conversion_type(std::int32_t, i32)
declare_conversion_type(std::int64_t, i64)
declare_conversion_type(std::uint8_t, u8)
declare_conversion_type(std::uint16_t, u16)
declare_conversion_type(std::uint32_t, u32)
declare_conversion_type(std::uint64_t, u64)
declare_conversion_type(float, r32)
declare_conversion_type(double, r64)
declare_conversion_type(long double, r96)
declare_conversion_type(bool, boolean)
declare_conversion_type(std::string, string)
declare_conversion_type(octet_string_t, octet_string)


template<typename T>
std::size_t get_value_size(const T &value)
{
    return detail::get_value_size(value);
}

}
