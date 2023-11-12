#ifndef MPL_OPTION_TYPES_H
#define MPL_OPTION_TYPES_H

#include <cstdint>
#include "tools/utils/any_base.h"

namespace mpl
{

using option_id_t = std::int64_t;
using option_value_t = pt::utils::any;

constexpr option_id_t opt_module_range_size = 0x00100000;

constexpr option_id_t opt_reserved_base = 0x00000000;
constexpr option_id_t opt_core_base     = 0x00100000;
constexpr option_id_t opt_other_base    = opt_core_base + opt_module_range_size;
constexpr option_id_t opt_user_base     = 0x10000000;

}

#endif // MPL_OPTION_TYPES_H
