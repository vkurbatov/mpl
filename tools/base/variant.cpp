#include "variant.h"
#include <cstring>
#include <sstream>
#include <sstream>
#include <limits>

namespace base
{

using string = std::string;
using int8 = std::int8_t;
using int16 = std::int16_t;
using int32 = std::int32_t;
using int64 = std::int64_t;
using uint8 = std::uint8_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;
using long_double = long double;
//using _double = double;


#define __define_block(block_name) \
    block_name(int8) \
    block_name(int16) \
    block_name(int32) \
    block_name(int64) \
    block_name(uint8) \
    block_name(uint16) \
    block_name(uint32) \
    block_name(uint64) \
    block_name(float) \
    block_name(double) \
    block_name(long_double) \
    block_name(string) \
    block_name(bool) \

#define __define_block_args(block_name, ...) \
    block_name(int8, __VA_ARGS__) \
    block_name(int16, __VA_ARGS__) \
    block_name(int32, __VA_ARGS__) \
    block_name(int64, __VA_ARGS__) \
    block_name(uint8, __VA_ARGS__) \
    block_name(uint16, __VA_ARGS__) \
    block_name(uint32, __VA_ARGS__) \
    block_name(uint64, __VA_ARGS__) \
    block_name(float, __VA_ARGS__) \
    block_name(double, __VA_ARGS__) \
    block_name(long_double, __VA_ARGS__) \
    block_name(string, __VA_ARGS__) \
    block_name(bool, __VA_ARGS__) \


template<typename T>
T max_value()
{
    return std::numeric_limits<T>::max();
}

template<typename T>
T min_value()
{
    return std::numeric_limits<T>::min();
}

template<typename T>
T from_string(const std::string& string_value
              , const T& default_value = {})
{
    T result_value = {};
    std::stringstream ss;

    ss << string_value;
    ss >> result_value;

    return ss.fail()
            ? default_value
            : result_value;
}

template<typename T>
std::string to_string(const T& value
                      , const std::string& default_value = {})
{
    /*
    std::string string_value = {};
    std::stringstream ss;

    ss << value;
    ss >> string_value;

    return ss.fail()
            ? default_value
            : string_value;*/
    return std::to_string(value);
}

template<typename Tin, typename Tout>
struct converter
{
    static Tout convert(const Tin& value
                        , const Tout& default_value= {})
    {
        return static_cast<Tout>(value);
    }
};

template<typename Tin>
struct converter<Tin, std::string>
{
    static std::string convert(const Tin& value
                               , const std::string& default_value = {})
    {
        return to_string(value
                         , default_value);
    }
};

template<typename Tout>
struct converter<std::string, Tout>
{
    static Tout convert(const std::string& value
                        , const Tout& default_value= {})
    {
        return from_string(value
                         , default_value);
    }
};

template<>
struct converter<std::string, std::string>
{
    static std::string convert(const std::string& value
                        , const std::string& default_value= {})
    {
        return value;
    }
};

template<typename T>
constexpr bool is_container()
{
    return false;
}

template<>
constexpr bool is_container<std::string>()
{
    return true;
}

template<typename T, bool C = is_container<T>()>
struct storage_serializer_t
{
    storage_serializer_t(storage_value_t& storage_value);
    void set(const T& value);
};

template<typename T>
struct storage_serializer_t<T, false>
{
    static constexpr auto value_size = sizeof(T);

    storage_value_t&    m_storage_value;

    storage_serializer_t(storage_value_t& storage_value)
        : m_storage_value(storage_value)
    {

    }

    void set(const T& value)
    {
        m_storage_value.resize(value_size);
        std::memcpy(m_storage_value.data()
                    , &value
                    , value_size);
    }
};

template<typename T>
struct storage_serializer_t<T, true>
{
    static const auto value_size = sizeof(typename T::value_type);

    storage_value_t&    m_storage_value;

    storage_serializer_t(storage_value_t& storage_value)
        : m_storage_value(storage_value)
    {

    }

    void set(const T& value)
    {
        m_storage_value.resize(value.size() * value_size);
        std::memcpy(m_storage_value.data()
                    , value.data()
                    , m_storage_value.size());
    }
};

template<typename T, bool C = is_container<T>()>
struct storage_deserializer_t
{
    storage_deserializer_t(const storage_value_t& storage_value);
    T get(const T& default_value = {});
};

template<typename T>
struct storage_deserializer_t<T, false>
{
    static constexpr auto value_size = sizeof(T);

    const storage_value_t&  m_storage_value;

    storage_deserializer_t(const storage_value_t& storage_value)
        : m_storage_value(storage_value)
    {

    }

    T get(const T& default_value = {})
    {
        if (m_storage_value.size() == value_size)
        {
            return reinterpret_cast<const T&>(*m_storage_value.data());
        }

        return default_value;
    }
};

template<typename T>
struct storage_deserializer_t<T, true>
{
    static constexpr auto value_size = sizeof(typename T::value_type);

    const storage_value_t&  m_storage_value;

    storage_deserializer_t(const storage_value_t& storage_value)
        : m_storage_value(storage_value)
    {

    }

    T get(const T& default_value = {})
    {
        auto result_value = default_value;

        if (!m_storage_value.empty())
        {
            result_value.resize(m_storage_value.size() / value_size);
            std::memcpy(&result_value[0]
                        , m_storage_value.data()
                        , m_storage_value.size());
        }

        return result_value;
    }
};

template<typename Tout>
Tout get_storage_value(const storage_value_t& strorage
                       , variant_type_t store_type
                       , const Tout& default_value={})
{
    switch(store_type)
    {
        #define __case_storage_value(type) \
        case variant_type_t::vt_##type: \
        { \
            return converter<type, Tout>::convert(storage_deserializer_t<type>(strorage).get() \
                                                  , default_value); \
        } \
        break;

        __define_block(__case_storage_value)
    }

    return default_value;
}


//-------------------------------------------------------------------------------------

#define __define_variant_constructor(type) \
template<> \
variant::variant(const type& value) \
    : m_variant_type(variant_type_t::vt_##type) \
{ \
    set<type>(value); \
}

#define __define_variant_setter(type) \
template<> \
void variant::set(const type& value) \
{ \
    m_variant_type = variant_type_t::vt_##type; \
    storage_serializer_t<type> serializer(m_storage); \
    serializer.set(value); \
} \
template<> \
variant& variant::operator=(const type& value) \
{ \
    set(value); \
    return *this; \
}

#define __define_variant_getter(type) \
template<> \
type variant::get(const type& default_value) const \
{ \
    return get_storage_value<type>(m_storage \
                                   , m_variant_type \
                                   , default_value); \
} \
template<> \
variant::operator type() const \
{ \
    return get<type>(); \
}

#define __define_variant_type(type) \
    __define_variant_setter(type) \
    __define_variant_getter(type) \
    __define_variant_constructor(type)

__define_block(__define_variant_type)

std::size_t variant::hasher_t::operator()(const variant &value) const
{
    return value.hash();
}

variant::variant()
    : m_variant_type(variant_type_t::vt_unknown)
{

}

variant::variant(const char *value)
    : variant(std::string(value))
{

}

void variant::set(const char *value)
{
    set<std::string>(value);
}

#define __case_compare(type, op) \
case variant_type_t::vt_##type: \
{ \
    return get<type>() op variant.get<type>(max_value<type>()); \
} \
break;

#define __define_compare_operator(op) \
bool variant::operator op (const variant &variant) const \
{ \
    switch(m_variant_type) \
    { \
        __define_block_args(__case_compare, op) \
    } \
    return false; \
}

__define_compare_operator(==)
__define_compare_operator(!=)
__define_compare_operator(>)
__define_compare_operator(>=)
__define_compare_operator(<)
__define_compare_operator(<=)

void variant::set_type(variant_type_t variant_type)
{
    if (variant_type != m_variant_type)
    {
        switch(variant_type)
        {
            #define __case_set_value(type) \
            case variant_type_t::vt_##type: \
            { \
                set(get<type>()); \
            } \
            break;

            __define_block(__case_set_value);
        }
    }
}

variant_type_t variant::type() const
{
    return m_variant_type;
}

std::size_t variant::hash() const
{
    std::size_t total_hash = 0;
    std::int32_t i = 0;
    for (const auto& d :m_storage)
    {
        total_hash ^= std::hash<std::uint8_t>()(d) << (i % sizeof(std::size_t)) * 8;
        i++;
    }

    return total_hash;
}

bool variant::is_empty() const
{
    return m_variant_type == variant_type_t::vt_unknown
            || m_storage.empty();
}


void test()
{
    return;
}


}
