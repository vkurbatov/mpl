#ifndef MPL_PACKET_TYPES_H
#define MPL_PACKET_TYPES_H

#include <cstdint>

namespace mpl
{

enum class field_type_t : std::uint8_t
{
    object_begin,
    numeric,
    real,
    string,
    object_end = 0x07
};

enum class packet_control_t
{
    begin,
    end
};

struct field_header_t
{
    field_type_t        type = field_type_t::object_begin;
    std::uint32_t       size = 0;
};

struct data_field_t : public field_header_t
{
    const void*         data = nullptr;
};

}

#endif // MPL_PACKET_TYPES_H
