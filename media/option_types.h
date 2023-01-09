#ifndef MPL_OPTION_TYPES_H
#define MPL_OPTION_TYPES_H

#include <cstdint>
#include "tools/base/any_base.h"

namespace mpl
{

using option_id_t = std::int64_t;
using option_value_t = base::any;


constexpr option_id_t opt_fmt_base = 0x0000000;
constexpr option_id_t opt_other_base = 0x00001000;


constexpr option_id_t opt_fmt_stream_id =       opt_fmt_base + 0;
constexpr option_id_t opt_fmt_device_id =       opt_fmt_base + 1;
constexpr option_id_t opt_fmt_extra_data =      opt_fmt_base + 2;
constexpr option_id_t opt_fmt_params =          opt_fmt_base + 3;

constexpr option_id_t user_base = 0x10000000;


}

#endif // MPL_OPTION_TYPES_H
