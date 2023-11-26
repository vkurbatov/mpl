#ifndef MPL_JSON_UTILS_H
#define MPL_JSON_UTILS_H

#include "core/i_property.h"

namespace mpl::utils
{

i_property::u_ptr_t from_json(const std::string& json_string);
std::string to_json(i_property& property, bool prettify = false);

}

#endif // MPL_JSON_UTILS_H
