#ifndef OCV_DRAW_FORMAT_H
#define OCV_DRAW_FORMAT_H

#include "font_format.h"

namespace ocv
{

struct draw_format_t
{
    font_format_t   font_format;
    color_t         font_color;
    color_t         pen_color;
    color_t         fill_color;
    double          draw_opacity = 1.0;
    std::int32_t    line_weight = 1;
};

}

#endif // OCV_DRAW_FORMAT_H
