#ifndef IO_SERIAL_TYPES_H
#define IO_SERIAL_TYPES_H

#include <cstdint>

namespace pt::io
{

using baud_rate_t = std::uint32_t;
using char_size_t = std::uint8_t;

constexpr static char_size_t min_char_size = 5;
constexpr static char_size_t max_char_size = 8;

enum class serial_stop_bits_t
{
    one,
    onepointfive,
    two
};

enum class serial_parity_t
{
    none,
    odd,
    even
};

enum class serial_flow_control_t
{
    none,
    software,
    hardware
};

}

#endif // IO_SERIAL_TYPES_H
