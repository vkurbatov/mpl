#ifndef MPL_ENUM_UTILS_H
#define MPL_ENUM_UTILS_H

#include <string>
#include <optional>

namespace mpl::core::utils
{

template<typename E>
std::string enum_to_string(const E& enum_value, const std::string& default_string = {});

template<typename E>
E string_to_enum(const std::string& enum_string, const E& default_value);

template<typename E>
std::optional<E> string_to_enum(const std::string& enum_string);

}

#endif // MPL_ENUM_UTILS_H
