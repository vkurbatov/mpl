#include "v4l2_utils.h"
#include "video_info.h"

#include <linux/videodev2.h>

namespace mpl::media::utils
{

video_format_id_t format_form_v4l2(v4l2::pixel_format_t v4l2_format)
{
    return video_format_info_t::format_from_fourcc(v4l2_format);
}

v4l2::pixel_format_t format_to_v4l2(video_format_id_t video_format)
{
    return video_format_info_t::get_info(video_format).fourcc;
}

}


