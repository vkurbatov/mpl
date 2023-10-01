#include "convert_utils.h"

#include "core/common_types.h"

#include "tools/base/convert_base.h"

#include <string>

namespace mpl::utils
{

#define declare_conversion_func(type_in, type_out)\
    template bool convert(const type_in&, type_out&);

#define declare_conversion_pair(type_in)\
    declare_conversion_func(type_in, std::int8_t)\
    declare_conversion_func(type_in, std::int16_t)\
    declare_conversion_func(type_in, std::int32_t)\
    declare_conversion_func(type_in, std::int64_t)\
    declare_conversion_func(type_in, std::uint8_t)\
    declare_conversion_func(type_in, std::uint16_t)\
    declare_conversion_func(type_in, std::uint32_t)\
    declare_conversion_func(type_in, std::uint64_t)\
    declare_conversion_func(type_in, float)\
    declare_conversion_func(type_in, double)\
    declare_conversion_func(type_in, long double)\
    declare_conversion_func(type_in, bool)\
    declare_conversion_func(type_in, std::string)\
    declare_conversion_func(type_in, octet_string_t)

declare_conversion_pair(std::int8_t)\
declare_conversion_pair(std::int16_t)\
declare_conversion_pair(std::int32_t)\
declare_conversion_pair(std::int64_t)\
declare_conversion_pair(std::uint8_t)\
declare_conversion_pair(std::uint16_t)\
declare_conversion_pair(std::uint32_t)\
declare_conversion_pair(std::uint64_t)\
declare_conversion_pair(float)\
declare_conversion_pair(double)\
declare_conversion_pair(long double)\
declare_conversion_pair(bool)\
declare_conversion_pair(std::string)\
declare_conversion_pair(octet_string_t)


template<typename Tin, typename Tout>
bool convert(const Tin& in_value, Tout& out_value)
{
    return base::convert(in_value
                         , out_value);
}

}
