#include "endian_utils.h"

#include "tools/base/endian_base.h"
#include <algorithm>
#include <cstring>

namespace mpl::utils::endian
{

using order_t = portable::octet_order_t;

namespace detail
{

template<order_t Order>
void* convert(void *value
              , std::size_t size)
{
    return portable::utils::convert_order(Order, value, size);
}

template<order_t Order, typename T>
T convert(const T& value)
{
    auto result = value;
    convert<Order>(&result, sizeof(T));
    return result;
}

template<order_t Order, typename T>
T get_value(const void* data
            , std::size_t size = sizeof(T)
            , std::int32_t offset = 0)
{
    data = static_cast<const std::uint8_t*>(data) + offset;
    size = std::min(size, sizeof(T));
    T value = {};
    std::memcpy(&value
                , data
                , size);
    convert<Order>(&value, size);
    return value;
}

template<order_t Order, typename T>
std::size_t set_value(const T& value
                       , void* data
                       , std::size_t size = sizeof(T)
                       , std::int32_t offset = 0)
{
    data = static_cast<std::uint8_t*>(data) + offset;
    size = std::min(size, sizeof(T));
    std::memcpy(data
                , &value
                , size);
    convert<Order>(data, size);

    return size;
}

}

#define __declare_endian_convert_type(order, type)\
    template<> type convert(const type& value) { return detail::convert<order>(value); } \
    template<> type get_value<type>(const void* data, std::size_t size, std::int32_t offset) { return detail::get_value<order, type>(data, size, offset); } \
    template<> std::size_t set_value<type>(const type& value, void* data, std::size_t size, std::int32_t offset) { return detail::set_value<order, type>(value, data, size, offset); }

#define __declare_endian_convert_order(order)\
    __declare_endian_convert_type(order, std::uint8_t)\
    __declare_endian_convert_type(order, std::uint16_t)\
    __declare_endian_convert_type(order, std::uint32_t)\
    __declare_endian_convert_type(order, std::uint64_t)\
    __declare_endian_convert_type(order, std::int8_t)\
    __declare_endian_convert_type(order, std::int16_t)\
    __declare_endian_convert_type(order, std::int32_t)\
    __declare_endian_convert_type(order, std::int64_t)\
    __declare_endian_convert_type(order, float)\
    __declare_endian_convert_type(order, double)\
    __declare_endian_convert_type(order, long double)\
    void* convert(void *value, std::size_t size) { return detail::convert<order>(value, size); }

namespace big
{
    __declare_endian_convert_order(portable::octet_order_t::big_endian)
}

namespace little
{
    __declare_endian_convert_order(portable::octet_order_t::little_endian)
}


}
