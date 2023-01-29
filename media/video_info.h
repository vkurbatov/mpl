#ifndef MPL_VIDEO_INFO_H
#define MPL_VIDEO_INFO_H

#include "video_types.h"
#include <cstdint>

namespace mpl::media
{

struct video_format_info_t
{
    static const video_format_info_t& get_info(video_format_id_t format_id);
    static video_format_id_t format_from_fourcc(std::uint32_t fourcc);


    std::int32_t    bpp;
    bool            encoded;
    bool            planar;
    bool            convertable;
    bool            motion;
    std::int32_t    align_widht;
    std::int32_t    align_height;
    std::uint32_t   fourcc;
};

}

#endif // MPL_VIDEO_INFO_H
