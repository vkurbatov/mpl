#include "bitstream_base.h"
#include <algorithm>
#include <bit>
#include <cstring>

namespace base
{

namespace utils
{

octet_order_t this_order()
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    return octet_order_t::little_endian;
#else
    return octet_order_t::big_endian;
#endif
}

void *convert_order(void *octet_string, std::size_t size)
{
    std::reverse(static_cast<std::uint8_t*>(octet_string)
                 , static_cast<std::uint8_t*>(octet_string) + size);
    return octet_string;
}

void *convert_order(octet_order_t ocetet_order, void *octet_string, std::size_t size)
{
    if (ocetet_order != this_octet_order)
    {
        convert_order(octet_string
                      , size);
    }

    return octet_string;
}

bool get_bit(const void *src_stream, int32_t bit_index)
{
    const auto src_ptr = static_cast<const std::uint8_t*>(src_stream);
    return (src_ptr[bit_index / 8] & (1 << (bit_index % 8))) != 0;
}

void set_bit(void *dst_stream, int32_t bit_index, bool value)
{
    auto dst_ptr = static_cast<std::uint8_t*>(dst_stream);
    if (value)
    {
        dst_ptr[bit_index / 8] |= (1 << (bit_index % 8));
    }
    else
    {
        dst_ptr[bit_index / 8] &= ~(1 << (bit_index % 8));
    }
}


void copy_bits(const void *src_stream
               , void* dst_stream
               , int32_t src_idx
               , int32_t dst_idx
               , std::size_t bits)
{
    const auto src_ptr = static_cast<const std::uint8_t*>(src_stream);
    auto dst_ptr = static_cast<std::uint8_t*>(dst_stream);

    if (src_idx % 8 == 0
        && dst_idx % 8 == 0
        && bits % 8 == 0)
    {
        std::memcpy(dst_ptr + dst_idx / 8
                    , src_ptr + src_idx / 8
                    , bits / 8);
        return;
    }

    while (bits-- > 0)
    {
        set_bit(dst_ptr, dst_idx, get_bit(src_ptr, src_idx));
        dst_idx++;
        src_idx++;
    }

}

}


}
