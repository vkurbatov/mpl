#ifndef BASE_CONVERT_BASE_H
#define BASE_CONVERT_BASE_H

#include <optional>
#include <cstdint>

namespace base
{

enum class value_type_t
{
    undefined = -1,
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

template<typename T>
value_type_t get_value_type();

template<typename T>
std::size_t get_value_size(const T& value);

template<typename Tin, typename Tout>
bool convert(const Tin& in, Tout& out);

template<typename Tin, typename Tout>
std::optional<Tout> convert(const Tin& in);

}

#endif // BASE_CONVERT_BASE_H
