#ifndef OCV_FONT_FORMAT_T_H
#define OCV_FONT_FORMAT_T_H

#include "ocv_types.h"

namespace pt::ocv
{

constexpr std::int32_t default_font_height = 8;
constexpr std::int32_t default_font_weight = 1;

enum class font_t
{
    simplex,
    plain,
    duplex,
    complex,
    triplex,
    complex_small,
    script_simplex,
    script_complex
};

struct font_format_t
{
    font_t          font;
    std::int32_t    height;
    std::int32_t    weight;
    bool            italic;

    font_format_t(const font_t& font = font_t::simplex
                  , std::int32_t height = default_font_height
                  , std::int32_t weight = default_font_weight
                  , bool italic = false);

    std::int32_t native_font() const;
    double scale_font() const;
    frame_size_t text_size(const std::string& text) const;

};

}

#endif // OCV_FONT_FORMAT_T_H
