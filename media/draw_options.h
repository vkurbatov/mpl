#ifndef MPL_MEDIA_DRAW_OPTIONS_H
#define MPL_MEDIA_DRAW_OPTIONS_H

#include "video_frame_types.h"
#include <string>

namespace mpl::media
{

struct draw_image_options_t
{
    relative_frame_rect_t   target_rect;
    double                  opacity;

    draw_image_options_t(const relative_frame_rect_t& target_rect = {}
                         , double opacity = 1.0);

    bool operator == (const draw_image_options_t& other) const;
    bool operator != (const draw_image_options_t& other) const;
};

}

#endif // MPL_MEDIA_DRAW_OPTIONS_H
