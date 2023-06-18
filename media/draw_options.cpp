#include "draw_options.h"

namespace mpl::media
{

draw_options_t::draw_options_t(const relative_frame_rect_t &target_rect
                               , double opacity
                               , std::int32_t border
                               , double margin
                               , const std::string& label)
    : target_rect(target_rect)
    , opacity(opacity)
    , border(border)
    , margin(margin)
    , label(label)
{

}

bool draw_options_t::operator ==(const draw_options_t &other) const
{
    return target_rect == other.target_rect
            && opacity == other.opacity
            && border == other.border
            && margin == other.margin
            && label == other.label;
}

bool draw_options_t::operator !=(const draw_options_t &other) const
{
    return ! operator == (other);
}

}
