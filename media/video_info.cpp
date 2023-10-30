#include "video_info.h"
#include "video_format_info.h"
#include "i_video_format.h"

namespace mpl::media
{

video_info_t::video_info_t(video_format_id_t format_id
                           , const frame_size_t &size
                           , double frame_rate)
    : format_id(format_id)
    , size(size)
    , frame_rate(frame_rate)
{

}

video_info_t::video_info_t(const i_video_format &video_format)
    : format_id(video_format.format_id())
    , size(video_format.width(), video_format.height())
    , frame_rate(video_format.frame_rate())
{

}

bool video_info_t::operator ==(const video_info_t &other) const
{
    return format_id == other.format_id
            && size == other.size
            && frame_rate == other.frame_rate;
}

bool video_info_t::operator !=(const video_info_t &other) const
{
    return ! operator == (other);
}

std::size_t video_info_t::bpp() const
{
    return video_format_info_t::get_info(format_id).bpp;
}

std::size_t video_info_t::frame_size() const
{
    return (size.size() * bpp()) / 8;
}

bool video_info_t::is_valid() const
{
    return frame_size() > 0;
}

}
