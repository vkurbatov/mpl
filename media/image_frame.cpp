#include "image_frame.h"
#include "video_info.h"

namespace mpl::media
{

image_frame_t::image_frame_t(video_format_id_t format_id
                             , const frame_size_t& size
                             , smart_buffer &&image_data)
    : format_id(format_id)
    , size(size)
    , image_data(std::move(image_data))
{

}

image_frame_t::image_frame_t(video_format_id_t format_id
                             , const frame_size_t &size
                             , const smart_buffer &image_data)
    : format_id(format_id)
    , size(size)
    , image_data(image_data)
{

}

bool image_frame_t::operator ==(const image_frame_t &other) const
{
    return format_id == other.format_id
            && size == other.size
            && image_data == other.image_data;
}

bool image_frame_t::operator !=(const image_frame_t &other) const
{
    return ! operator == (other);
}

std::size_t image_frame_t::frame_size() const
{
    return (size.size() * video_format_info_t::get_info(format_id).bpp) / 8;
}

const void *image_frame_t::pixels() const
{
    return image_data.data();
}

void *image_frame_t::pixels()
{
    return image_data.map();
}

bool image_frame_t::tune()
{
    if (video_format_info_t::get_info(format_id).convertable)
    {
        if (auto fsize = frame_size())
        {
            if (fsize != image_data.size())
            {
                image_data.resize(fsize);
            }
        }
    }

    return false;
}

bool image_frame_t::is_valid() const
{
   return video_format_info_t::get_info(format_id).convertable
           && frame_size() <= image_data.size();
}

bool image_frame_t::is_empty() const
{
    return image_data.is_empty();
}

}
