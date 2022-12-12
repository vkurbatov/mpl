#include "ocv_utils.h"

namespace ocv::utils
{

const format_info_t &get_format_info(const frame_format_t& format)
{
    static const format_info_t format_table[] =
    {
        { "UNK",    0  },
        { "BGR",    24 },
        { "BGRA",   32 }
    };

    return format_table[static_cast<std::int32_t>(format) + 1];
}

}
