#include "utils/enum_converter_defs.h"

#include "log_types.h"

namespace mpl::utils
{

using namespace log;

__declare_enum_converter_begin(log_level_t)
    __declare_enum_pair(log_level_t, trace),
    __declare_enum_pair(log_level_t, debug),
    __declare_enum_pair(log_level_t, info),
    __declare_enum_pair(log_level_t, warning),
    __declare_enum_pair(log_level_t, error),
    __declare_enum_pair(log_level_t, fatal),
__declare_enum_converter_end(log_level_t)

}
