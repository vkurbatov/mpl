#ifndef MPL_FORMAT_UTILS_H
#define MPL_FORMAT_UTILS_H

#include <string>

namespace mpl::utils
{

std::uint32_t to_fourcc(const std::string_view& format, bool be = false);

}

#endif // MPL_FORMAT_UTILS_H
