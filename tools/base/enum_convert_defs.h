#ifndef BASE_ENUM_CONVERT_DEFS_H
#define BASE_ENUM_CONVERT_DEFS_H

#define __begin_declare_enum_conversion(enum_type)\
static const std::string table_##enum_type[] = \

#define __end_declare_enum_conversion(enum_type, offset)\
;\
template<>\
const std::string& base::enum_to_string(const enum_type& enum_value)\
{\
    return table_##enum_type[static_cast<std::int32_t>(enum_value) + offset];\
}\
template<>\
bool base::string_to_enum(const std::string& enum_string, enum_type& enum_value)\
{\
    auto idx = 0;\
    for (const auto& s : table_##enum_type)\
    {\
        if (s == enum_string)\
        {\
            enum_value = static_cast<enum_type>(idx - offset);\
            return true;\
        }\
        idx++;\
    }\
    return false;\
}\
template<>\
enum_type base::string_to_enum(const std::string& enum_string, const enum_type& default_enum_value)\
{\
    enum_type return_value = default_enum_value;\
    string_to_enum(enum_string, return_value);\
    return return_value;\
}

#endif // BASE_ENUM_DIFINES_H
