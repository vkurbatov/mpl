#ifndef OCV_IMAGE_H
#define OCV_IMAGE_H

#include "frame_info.h"
#include <vector>

#include <opencv2/imgproc.hpp>

namespace ocv
{

struct image_t
{
    using raw_data_t = std::vector<std::uint8_t>;

    frame_info_t    info;
    raw_data_t      data;

    image_t(const frame_info_t& info = {}
            , raw_data_t&& data = {});

    image_t(const frame_info_t& info
            , const raw_data_t& data);

    bool is_valid() const;
    bool is_empty() const;

    bool load(const std::string& file_path
              , frame_format_t format = frame_format_t::bgr);

    bool resize(const frame_size_t& new_size);


};

}

#endif // OCV_IMAGE_H
