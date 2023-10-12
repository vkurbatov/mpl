#ifndef BASE_ENUM_UTILS_H
#define BASE_ENUM_UTILS_H

#include <string>

namespace portable
{

template<typename T>
const std::string& enum_to_string(const T&);

template<typename T>
bool string_to_enum(const std::string&, T&);

template<typename T>
T string_to_enum(const std::string&, const T&);

}

#endif // BASE_ENUM_UTILS_H
