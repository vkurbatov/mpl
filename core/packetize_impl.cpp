#include "packetizer.h"
#include "depacketizer.h"

#include <string>
#include <vector>

#include <cstring>


namespace mpl
{

namespace detail
{

template<typename T>
bool set_value(const void* data, std::size_t size, T& value)
{
    if (size <= sizeof(T))
    {
        value = {};
        std::memcpy(&value
                    , data
                    , size);
        return true;
    }
    return false;
}

template<>
bool set_value(const void* data, std::size_t size, std::string& value)
{
    value = std::string(static_cast<const char*>(data), size);
    return true;
}

template<typename T>
std::size_t clamped_size(const T& value)
{
    std::size_t result = sizeof(T);

    while(result > 0)
    {
        if (*(reinterpret_cast<const std::uint8_t*>(&value) + result - 1) != 0)
        {
            break;
        }
        result--;
    }

    return result;
}

template<typename T>
field_type_t get_field_type();

#define __declare_field_type(type, ftype) template<> field_type_t get_field_type<type>() { return field_type_t::ftype; }

__declare_field_type(std::int8_t, numeric)
__declare_field_type(std::int16_t, numeric)
__declare_field_type(std::int32_t, numeric)
__declare_field_type(std::int64_t, numeric)
__declare_field_type(std::uint8_t, numeric)
__declare_field_type(std::uint16_t, numeric)
__declare_field_type(std::uint32_t, numeric)
__declare_field_type(std::uint64_t, numeric)
__declare_field_type(bool, numeric)

__declare_field_type(float, real)
__declare_field_type(double, real)
__declare_field_type(long double, real)

__declare_field_type(std::string, string)
__declare_field_type(char*, string)

}

#define __declare_packetize_type(type)\
    template bool packetizer::add_value(const type& value);\
    template bool depacketizer::fetch_value(type& value);

__declare_packetize_type(std::int8_t)
__declare_packetize_type(std::int16_t)
__declare_packetize_type(std::int32_t)
__declare_packetize_type(std::int64_t)
__declare_packetize_type(std::uint8_t)
__declare_packetize_type(std::uint16_t)
__declare_packetize_type(std::uint32_t)
__declare_packetize_type(std::uint64_t)
__declare_packetize_type(bool)
__declare_packetize_type(float)
__declare_packetize_type(double)
__declare_packetize_type(long double)

// simple types
template<typename T>
bool packetizer::add_value(const T &value)
{
    return add(detail::get_field_type<T>()
                , &value
                , detail::clamped_size(value));


}

template<typename T>
bool depacketizer::fetch_value(T& value)
{
    return fetch(detail::get_field_type<T>()
                 , &value
                 , sizeof(T));
}

// vector of simple types
template<typename T>
bool packetizer::add_value(const std::vector<T>& value)
{
    return add(detail::get_field_type<T>()
                , value.data()
                , value.size() * sizeof(T));
}

template<typename T>
bool depacketizer::fetch_value(std::vector<T>& value)
{
    auto save_cursor = cursor();
    data_field_t data_field;

    if (fetch(data_field)
            && data_field.type == detail::get_field_type<T>())
    {
        value.resize(data_field.size / sizeof(T));
        std::memcpy(value.data()
                    , data_field.data
                    , value.size() * sizeof(T));

        return true;

    }

    seek(save_cursor);
    return false;
}

// string
template<>
bool packetizer::add_value(const std::string& value)
{
    return add(field_type_t::string
                , value.data()
                , value.size());


}

template<>
bool depacketizer::fetch_value(std::string& value)
{
    auto save_cursor = cursor();

    data_field_t data_field;
    if (fetch(data_field)
            && data_field.type == field_type_t::string)
    {
        value.resize(data_field.size);
        std::memcpy(value.data()
                    , data_field.data
                    , value.size());

        return true;
    }

    seek(save_cursor);
    return false;
}

// vector string
template<>
bool packetizer::add_value(const std::vector<std::string>& value)
{
    if (open_object())
    {
        for (const auto& v : value)
        {
            add(field_type_t::string
                , v.data()
                , v.size());
        }

        return close_object();
    }

    return false;
}

template<>
bool depacketizer::fetch_value(std::vector<std::string>& value)
{
    auto save_cursor = cursor();

    if (open_object())
    {
        data_field_t data_field = {};
        while(!fetch(data_field))
        {
            switch(data_field.type)
            {
                case field_type_t::string:
                    value.emplace_back(static_cast<const char*>(data_field.data)
                                       , data_field.size);
                    continue;
                break;
                case field_type_t::object_end:
                    return true;
                break;
                default:;
            }
            break;
        }
    }

    seek(save_cursor);
    return false;
}


}
