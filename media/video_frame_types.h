#ifndef MPL_VIDEO_FRAME_TYPES_H
#define MPL_VIDEO_FRAME_TYPES_H

#include "tools/utils/frame_base.h"

namespace mpl::media
{

using frame_point_t = pt::utils::frame_point_t;
using frame_size_t = pt::utils::frame_size_t;
using frame_rect_t = pt::utils::frame_rect_t;

using relative_frame_point_t = pt::utils::frame_point_float_t;
using relative_frame_size_t = pt::utils::frame_size_float_t;
using relative_frame_rect_t = pt::utils::frame_rect_float_t;

}

#endif // MPL_VIDEO_FRAME_TYPES_H
