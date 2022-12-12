#ifndef BASE_VARIANT_H
#define BASE_VARIANT_H

#include <string>
#include <vector>

namespace base
{

enum class variant_type_t
{
    vt_unknown,
    vt_int8,
    vt_int16,
    vt_int32,
    vt_int64,
    vt_uint8,
    vt_uint16,
    vt_uint32,
    vt_uint64,
    vt_float,
    vt_double,
    vt_long_double,
    vt_string,
    vt_bool
};

typedef std::vector<std::uint8_t> storage_value_t;

class variant
{
    storage_value_t     m_storage;
    variant_type_t      m_variant_type;

public:
    struct hasher_t
    {
        std::size_t operator()(const variant& value) const;
    };

    variant();

    template<typename T>
    variant(const T& value);

    variant(const char* value);

    template<typename T>
    void set(const T& value);

    template<typename T>
    variant& operator=(const T& value);

    void set(const char* value);

    bool operator == (const variant& variant_value) const;
    bool operator != (const variant& variant_value) const;
    bool operator > (const variant& variant_value) const;
    bool operator >= (const variant& variant_value) const;
    bool operator < (const variant& variant_value) const;
    bool operator <= (const variant& variant_value) const;

    template<typename T>
    T get(const T& default_value = {}) const;

    template<typename T>
    operator T() const;

    void set_type(variant_type_t variant_type);
    variant_type_t type() const;

    std::size_t hash() const;

    bool is_empty() const;

};

void test();

}


#endif // BASE_VARIANT_H
