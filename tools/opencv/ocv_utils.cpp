#include "ocv_utils.h"

#include <opencv2/core.hpp>

namespace pt::ocv::utils
{

const format_info_t &get_format_info(const frame_format_t& format)
{
    static const format_info_t format_table[] =
    {
        { "UNK",    0,  0},
        { "BGR",    24, CV_8UC3 },
        { "BGRA",   32, CV_8UC4},
        { "RGB",    24, CV_8UC3},
        { "RGBA",   32, CV_8UC4}
    };

    return format_table[static_cast<std::int32_t>(format) + 1];
}

}
