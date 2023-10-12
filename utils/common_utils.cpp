#include "common_utils.h"
#include "core/i_property.h"

#include "core/common_types.h"

#include <string>
#include "tools/base/convert_base.h"

namespace mpl::utils
{

namespace detail
{

template<typename T>
std::size_t get_value_size(const T& value)
{
    return portable::get_value_size(value);
}

template<>
std::size_t get_value_size(const i_property::s_array_t& value)
{
    return value.size();
}

}

#define declare_property_type(type, ptype) template<> property_type_t get_property_type<type>() { return property_type_t::ptype; }\
    template<> std::size_t get_value_size(const type& value) { return detail::get_value_size(value); };

declare_property_type(std::int8_t, i8)
declare_property_type(std::int16_t, i16)
declare_property_type(std::int32_t, i32)
declare_property_type(std::int64_t, i64)
declare_property_type(std::uint8_t, u8)
declare_property_type(std::uint16_t, u16)
declare_property_type(std::uint32_t, u32)
declare_property_type(std::uint64_t, u64)
declare_property_type(float, r32)
declare_property_type(double, r64)
declare_property_type(long double, r96)
declare_property_type(bool, boolean)
declare_property_type(std::string, string)
declare_property_type(octet_string_t, octet_string)
declare_property_type(i_property::s_array_t, array)

raw_array_t create_raw_array(const void *ref_data
                             , std::size_t ref_size)
{
    if (ref_data == nullptr)
    {
        return raw_array_t(ref_size);
    }

    return raw_array_t(static_cast<const raw_array_t::value_type*>(ref_data)
                        , static_cast<const raw_array_t::value_type*>(ref_data) + ref_size / sizeof(raw_array_t::value_type));
}

}
