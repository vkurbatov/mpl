#include "draw_options.h"

namespace mpl::media
{

draw_options_t::draw_options_t(const relative_frame_rect_t &target_rect
                               , double opacity
                               , double margin)
    : target_rect(target_rect)
    , opacity(opacity)
    , margin(margin)
{

}

bool draw_options_t::operator ==(const draw_options_t &other) const
{
    return target_rect == other.target_rect
            && opacity == other.opacity
            && margin == other.margin;
}

bool draw_options_t::operator !=(const draw_options_t &other) const
{
    return ! operator == (other);
}

}
