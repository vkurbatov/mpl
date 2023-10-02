#ifndef MPL_MEDIA_V4L2_UTILS_H
#define MPL_MEDIA_V4L2_UTILS_H

#include "video_types.h"
#include "tools/v4l2/v4l2_base.h"

namespace mpl::utils
{

media::video_format_id_t format_form_v4l2(v4l2::pixel_format_t v4l2_format);
v4l2::pixel_format_t format_to_v4l2(media::video_format_id_t video_format);

}

#endif // MPL_MEDIA_V4L2_UTILS_H
