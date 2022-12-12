#ifndef BASE_BITSTREAM_BASE_H
#define BASE_BITSTREAM_BASE_H

#include <cstdint>

namespace base
{

enum class octet_order_t
{
    big_endian,
    little_endian
};


namespace utils
{

octet_order_t this_order();
void* convert_order(void* octet_string, std::size_t size);
void* convert_order(octet_order_t ocetet_order, void* octet_string, std::size_t size);

template<typename T>
T convert_order(T value)
{
    return *static_cast<T*>(convert_order(&value, sizeof(T)));
}

template<typename T>
T convert_order(octet_order_t ocetet_order, T value)
{
    return *static_cast<T*>(convert_order(ocetet_order, &value, sizeof(T)));
}

bool get_bit(const void* src_stream
             , std::int32_t src_bit_index);

void set_bit(void* dst_stream
             , std::int32_t bit_index
             , bool value);

void copy_bits(const void* src_stream
               , void* dst_stream
               , std::int32_t src_idx
               , std::int32_t dst_idx
               , std::size_t bits);

}

static const octet_order_t this_octet_order = utils::this_order();

class bitstream_reader
{

};

class bitstream_writer
{

};

}

#endif // BASE_BITSTREAM_BASE_H
