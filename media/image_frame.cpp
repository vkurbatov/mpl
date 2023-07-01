#include "image_frame.h"
#include "video_info.h"

#include "i_video_format.h"

#include "tools/opencv/image.h"

namespace mpl::media
{

namespace detail
{

inline ocv::frame_format_t get_ocv_format(video_format_id_t format_id)
{
    switch(format_id)
    {
        case video_format_id_t::bgr24:
            return ocv::frame_format_t::bgr;
        break;
        case video_format_id_t::bgra32:
            return ocv::frame_format_t::bgra;
        break;
        case video_format_id_t::rgb24:
            return ocv::frame_format_t::rgb;
        break;
        case video_format_id_t::rgba32:
            return ocv::frame_format_t::rgba;
        break;
        default:;
    }

    return ocv::frame_format_t::undefined;
}

}


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

image_frame_t::image_frame_t(const image_info_t& image_info
                             , smart_buffer &&image_data)
    : image_info(image_info)
    , image_data(std::move(image_data))
{

}

image_frame_t::image_frame_t(const image_info_t& image_info
                             , const smart_buffer &image_data)
    : image_info(image_info)
    , image_data(image_data)
{

}

bool image_frame_t::operator ==(const image_frame_t &other) const
{
    return image_info == other.image_info
            && image_data == other.image_data;
}

bool image_frame_t::operator !=(const image_frame_t &other) const
{
    return ! operator == (other);
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
    if (image_info.is_valid())
    {
        if (auto fsize = image_info.frame_size())
        {
            if (fsize != image_data.size())
            {
                image_data.resize(fsize);
            }
        }
    }

    return false;
}

bool image_frame_t::blackout()
{
    if (!image_data.is_empty())
    {
        std::memset(image_data.map()
                    , 0
                    , image_data.size());
        return true;
    }

    return false;
}

bool image_frame_t::is_valid() const
{
   return image_info.is_valid()
           && image_info.frame_size() <= image_data.size();
}

bool image_frame_t::is_empty() const
{
    return image_data.is_empty();
}

bool image_frame_t::load(const std::string &path
                         , video_format_id_t format_id)
{
    ocv::image_t image;
    if (!path.empty()
            && image.load(path, detail::get_ocv_format(format_id))
            && image.is_valid())
    {
        this->image_info.format_id = format_id;
        image_info.size = image.info.size;
        image_data.assign(std::move(image.data));

        return true;
    }

    return false;
}


}
