#ifndef MPL_CONVERT_UTILS_H
#define MPL_CONVERT_UTILS_H

namespace mpl::core::utils
{

template<typename Tin, typename Tout>
bool convert(const Tin& in_value, Tout& out_value);

}

#endif // MPL_CONVERT_UTILS_H
