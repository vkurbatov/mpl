#include "font_format.h"
#include <opencv2/imgproc.hpp>

namespace ocv
{

font_format_t::font_format_t(const font_t &font
                             , int32_t height
                             , int32_t weight
                             , bool italic)
    : font(font)
    , height(height)
    , weight(weight)
    , italic(italic)
{

}

int32_t font_format_t::native_font() const
{
    return static_cast<cv::HersheyFonts>(font)
            | italic
            ? cv::HersheyFonts::FONT_ITALIC
            : 0;
}

double font_format_t::scale_font() const
{
    return cv::getFontScaleFromHeight(native_font()
                                       , height
                                       , weight);
}

frame_size_t font_format_t::text_size(const std::string &text) const
{
    std::int32_t y_line = 0;
    auto cv_size = cv::getTextSize(text
                                  , native_font()
                                  , scale_font()
                                  , weight
                                  , &y_line);

    return { cv_size.width, cv_size.height };
}

}
