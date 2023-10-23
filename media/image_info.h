#ifndef MPL_MEDIA_IMAGE_INFO_H
#define MPL_MEDIA_IMAGE_INFO_H

#include "video_types.h"
#include "video_frame_types.h"

namespace mpl::media
{

class i_video_format;

struct image_info_t
{
    video_format_id_t   format_id;
    frame_size_t        size;

    image_info_t(video_format_id_t format_id = video_format_id_t::undefined
                 , const frame_size_t& size = {});

    image_info_t(const i_video_format& video_format);

    bool operator == (const image_info_t& other) const;
    bool operator != (const image_info_t& other) const;

    std::size_t bpp() const;
    std::size_t frame_size() const;

    bool is_valid() const;
};

}

#endif // MPL_MEDIA_IMAGE_INFO_H
