#ifndef MPL_MEDIA_VIDEO_INFO_H
#define MPL_MEDIA_VIDEO_INFO_H

#include "video_types.h"
#include "video_frame_types.h"

#include <string>

namespace mpl::media
{

class i_video_format;

struct video_info_t
{
    video_format_id_t   format_id;
    frame_size_t        size;
    double              frame_rate;

    video_info_t(video_format_id_t format_id = video_format_id_t::undefined
                 , const frame_size_t& size = {}
                 , double frame_rate = 0.0);

    video_info_t(const i_video_format& video_format);

    bool operator == (const video_info_t& other) const;
    bool operator != (const video_info_t& other) const;

    std::size_t bpp() const;
    std::size_t frame_size() const;

    bool is_valid() const;
    bool is_compatible(const video_info_t& other) const;

    std::string to_string() const;
};

}

#endif // MPL_MEDIA_VIDEO_INFO_H
