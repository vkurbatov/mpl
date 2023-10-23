#include "image_info.h"
#include "video_info.h"
#include "i_video_format.h"

namespace mpl::media
{

image_info_t::image_info_t(video_format_id_t format_id
                           , const frame_size_t &size)
    : format_id(format_id)
    , size(size)
{

}

image_info_t::image_info_t(const i_video_format &video_format)
    : format_id(video_format.format_id())
    , size(video_format.width(), video_format.height())
{

}

bool image_info_t::operator ==(const image_info_t &other) const
{
    return format_id == other.format_id
            && size == other.size;
}

bool image_info_t::operator !=(const image_info_t &other) const
{
    return ! operator == (other);
}

std::size_t image_info_t::bpp() const
{
    return video_format_info_t::get_info(format_id).bpp;
}

std::size_t image_info_t::frame_size() const
{
    return (size.size() * bpp()) / 8;
}

bool image_info_t::is_valid() const
{
    return frame_size() > 0;
}

}
