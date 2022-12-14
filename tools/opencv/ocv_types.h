#ifndef OCV_TYPES_H
#define OCV_TYPES_H

#include "tools/base/frame_base.h"

#include <string>
#include <vector>

namespace ocv
{

namespace vars
{
    constexpr std::size_t color_depth = 8;
}

using frame_point_t = base::frame_point_t;
using frame_point_list_t = std::vector<frame_point_t>;
using frame_size_t = base::frame_size_t;
using frame_rect_t = base::frame_rect_t;
using frame_data_t = std::vector<std::uint8_t>;

using color_t = std::uint32_t;

enum class frame_format_t
{
    undefined = -1,
    bgr,
    bgra
};

enum class draw_figure_t
{
    rectangle,
    ellipse
};

struct format_info_t
{
    std::string name;
    std::size_t bits_per_second = 0;
};

}

#endif // OCV_TYPES_H
