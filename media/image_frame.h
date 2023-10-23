﻿#ifndef MPL_MEDIA_IMAGE_FRAME_H
#define MPL_MEDIA_IMAGE_FRAME_H

#include "video_types.h"
#include "video_frame_types.h"
#include "image_info.h"

#include "utils/smart_buffer.h"

#include <string>

namespace mpl::media
{

struct image_frame_t
{
    image_info_t            image_info;
    smart_buffer            image_data;

    image_frame_t(const image_info_t& image_info = {}
                 , smart_buffer&& image_data = {});

    image_frame_t(const image_info_t& image_info
                 , const smart_buffer& image_data);

    bool operator == (const image_frame_t& other) const;
    bool operator != (const image_frame_t& other) const;

    const void* pixels() const;
    void* pixels();

    bool tune();
    bool blackout();

    bool is_valid() const;
    bool is_empty() const;

    bool load(const std::string& path
              , video_format_id_t format_id = video_format_id_t::rgb24);
};

}

#endif // MPL_MEDIA_IMAGE_FRAME_H
