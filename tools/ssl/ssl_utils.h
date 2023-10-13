#ifndef SSL_UTILS_H
#define SSL_UTILS_H

#include "ssl_pointers.h"
#include <string>

namespace ssl
{

template<typename T, class ...Args>
ssl_s_ptr_t<T> create_object(Args ...args);

template<typename T, class ...Args>
ssl_unique_s_ptr_t<T> create_unique_object(Args ...args);

template<typename Method, typename Enum>
const Method* get_method(Enum method);

template<typename T>
T& set_enum_flag(T& flags, T flag, bool enable);

template<typename T>
bool get_enum_flag(const T& flags, T flag);

template<typename T>
std::string to_string(const T& value);

template<typename T>
bool from_string(const std::string& string_value, T& value);

template<typename Tout, typename Tin, class ...Args>
Tout convert(const Tin, Args ...args);

}

#endif // SSL_UTILS_H
