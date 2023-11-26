#ifndef PORTABLE_ENDIAN_BASE_H
#define PORTABLE_ENDIAN_BASE_H

#include <cstdint>
#include <type_traits>

namespace pt
{

namespace utils
{

enum class octet_order_t
{
    big_endian,
    little_endian
};

octet_order_t this_order();
void* convert_order(void* octet_string, std::size_t size);
void* convert_order(octet_order_t ocetet_order
                    , void* octet_string
                    , std::size_t size);

void copy(octet_order_t ocetet_order
          , void *dst_data
          , const void* src_data
          , std::size_t size
          , std::size_t offset = 0);

template<typename T>
T convert_order_value(T value, std::size_t size = sizeof(T))
{
    return *static_cast<T*>(convert_order(&value, size));
}

template<typename T>
T convert_order_value(octet_order_t ocetet_order, T value, std::size_t size = sizeof(T))
{
    return *static_cast<T*>(convert_order(ocetet_order, &value, size));
}

}

}

#endif // PORTABLE_ENDIAN_BASE_H
