#ifndef MPL_MEDIA_IMAGE_FRAME_H
#define MPL_MEDIA_IMAGE_FRAME_H

#include "video_types.h"
#include "video_frame_types.h"
#include "core/smart_buffer.h"

namespace mpl::media
{

struct image_frame_t
{
    video_format_id_t       format_id;
    frame_size_t            size;
    smart_buffer            image_data;

    image_frame_t(video_format_id_t format_id = video_format_id_t::undefined
                 , const frame_size_t& size = {}
                 , smart_buffer&& image_data = {});

    image_frame_t(video_format_id_t format_id
                 , const frame_size_t& size
                 , const smart_buffer& image_data);

    bool operator == (const image_frame_t& other) const;
    bool operator != (const image_frame_t& other) const;

    std::size_t frame_size() const;

    const void* pixels() const;
    void* pixels();

    bool tune();

    bool is_valid() const;
    bool is_empty() const;
};

}

#endif // MPL_MEDIA_IMAGE_FRAME_H
