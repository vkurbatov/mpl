#include "endian_base.h"

#include <algorithm>
#include <endian.h>

#include <cstring>

namespace portable
{

namespace utils
{

namespace detail
{

inline octet_order_t this_order()
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    return octet_order_t::little_endian;
#else
    return octet_order_t::big_endian;
#endif
}


}

octet_order_t this_order()
{
    return detail::this_order();
}

void copy(octet_order_t ocetet_order
          , void *dst_data
          , const void* src_data
          , std::size_t size
          , std::size_t offset)
{
    std::memcpy(dst_data
                , static_cast<const std::uint8_t*>(src_data) + offset
                , size);

    if (ocetet_order != this_order())
    {
        convert_order(dst_data
                      , size);
    }
}

void *convert_order(void *octet_string, std::size_t size)
{
    std::reverse(static_cast<std::uint8_t*>(octet_string)
                 , static_cast<std::uint8_t*>(octet_string) + size);
    return octet_string;
}

void *convert_order(octet_order_t ocetet_order, void *octet_string, std::size_t size)
{
    if (ocetet_order != detail::this_order())
    {
        convert_order(octet_string
                      , size);

    }

    return octet_string;
}

}

}
