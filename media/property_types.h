#ifndef MPL_PROPERTY_TYPES_H
#define MPL_PROPERTY_TYPES_H

namespace mpl
{

enum class property_type_t
{
    undefined = -1,
    object,
    array,
    i8,
    i16,
    i32,
    i64,
    u8,
    u16,
    u32,
    u64,
    r32,
    r64,
    r96,
    boolean,
    string,
    octet_string
};

}

#endif // MPL_PROPERTY_TYPES_H
