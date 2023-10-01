#include "v4l2_utils.h"
#include "video_info.h"

#include "video_format_impl.h"

#include "utils/convert_utils.h"

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

namespace mpl::utils
{


template<>
bool convert(const v4l2::frame_info_t& frame_info
             , media::video_format_impl& video_format)
{
    auto format_id = media::utils::format_form_v4l2(frame_info.pixel_format);
    if (format_id != media::video_format_id_t::undefined)
    {
        video_format.set_format_id(format_id);
        video_format.set_width(frame_info.size.width);
        video_format.set_height(frame_info.size.height);
        video_format.set_frame_rate(frame_info.fps);

        return true;
    }

    return false;
}

template<>
bool convert(const media::i_video_format& video_format
             , v4l2::frame_info_t& frame_info)
{
    auto v4l2_pixel_format = media::utils::format_to_v4l2(video_format.format_id());
    if (v4l2_pixel_format != v4l2::pixel_format_unknown)
    {
        frame_info.pixel_format = v4l2_pixel_format;
        frame_info.size.width = video_format.width();
        frame_info.size.height = video_format.height();
        frame_info.fps = video_format.frame_rate();

        return true;
    }

    return false;
}

}


