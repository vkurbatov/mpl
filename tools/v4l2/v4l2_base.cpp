#include "v4l2_base.h"
#include <linux/videodev2.h>

namespace v4l2
{

const pixel_format_t pixel_format_h264 = V4L2_PIX_FMT_H264;
const pixel_format_t pixel_format_jpeg = V4L2_PIX_FMT_JPEG;
const pixel_format_t pixel_format_mjpeg = V4L2_PIX_FMT_MJPEG;

const std::uint32_t ctrl_tilt_absolute = V4L2_CID_TILT_ABSOLUTE;
const std::uint32_t ctrl_pan_absolute = V4L2_CID_PAN_ABSOLUTE;
const std::uint32_t ctrl_zoom_absolute = V4L2_CID_ZOOM_ABSOLUTE;
const std::uint32_t ctrl_pan_speed = V4L2_CID_PAN_SPEED;
const std::uint32_t ctrl_tilt_speed = V4L2_CID_TILT_SPEED;
const std::uint32_t ctrl_zoom_speed = V4L2_CID_ZOOM_CONTINUOUS;


frame_size_t::frame_size_t(uint32_t width
                           , uint32_t height)
    : width(width)
    , height(height)
{

}

bool frame_size_t::operator ==(const frame_size_t &frame_size) const
{
    return width == frame_size.width
            && height == frame_size.height;
}

bool frame_size_t::operator !=(const frame_size_t &frame_size) const
{
    return !operator ==(frame_size);
}

bool frame_size_t::is_null() const
{
    return width == 0
            || height == 0;
}

frame_info_t::frame_info_t(const frame_size_t &size
                           , uint32_t fps
                           , pixel_format_t pixel_format)
    : size(size)
    , fps(fps)
    , pixel_format(pixel_format)
{

}

bool frame_info_t::operator ==(const frame_info_t &frame_info) const
{
    return size == frame_info.size
            && fps == frame_info.fps
            && pixel_format == frame_info.pixel_format;
}

bool frame_info_t::operator !=(const frame_info_t &frame_info) const
{
    return ! operator ==(frame_info);
}

bool frame_info_t::is_null() const
{
    return size.is_null()
            || pixel_format == 0;
}

frame_t::frame_t(const frame_info_t &frame_info
                 , const frame_data_t &frame_data)
    : frame_info(frame_info)
    , frame_data(frame_data)
{

}

frame_t::frame_t(const frame_info_t &frame_info
                 , frame_data_t &&frame_data)
    : frame_info(frame_info)
    , frame_data(frame_data)
{

}

control_menu_t::control_menu_t(uint32_t id, const std::string &name)
    : id(id)
    , name(name)
{

}


control_range_t::control_range_t(ctrl_value_t min
                                 , ctrl_value_t max)
    : min(min)
    , max(max)
{

}

bool control_range_t::operator ==(const control_range_t &range) const
{
    return min == range.min
            && max == range.max;
}

bool control_range_t::operator !=(const control_range_t &range) const
{
    return !operator == (range);
}

ctrl_value_t control_range_t::range_length() const
{
    return max - min;
}

bool control_range_t::is_join(ctrl_value_t value) const
{
    return value >= min && value <= max;
}

control_info_t::control_info_t(ctrl_id_t id
                     , const std::string &name
                     , ctrl_value_t step
                     , ctrl_value_t default_value
                     , ctrl_value_t current_value
                     , ctrl_value_t min
                     , ctrl_value_t max)
    : id(id)
    , name(name)
    , step(step)
    , default_value(default_value)
    , current_value(current_value)
    , range(min, max)
{

}

control_type_t control_info_t::type() const
{
    if (id != 0)
    {
        if (!menu.empty())
        {
            return control_type_t::menu;
        }

        if (range.min == 0
                && range.max == 1)
        {
            return control_type_t::boolean;
        }

        return control_type_t::numeric;
    }

    return control_type_t::undefined;
}

std::vector<std::string> control_info_t::menu_list() const
{
    std::vector<std::string> names;
    for (const auto& m : menu)
    {
        names.emplace_back(m.name);
    }
    return names;
}

const control_menu_t *control_info_t::get_menu_item(ctrl_id_t id) const
{
    for (const auto& m : menu)
    {
        if (m.id == id)
        {
            return &m;
        }
    }
    return nullptr;
}

const control_menu_t *control_info_t::get_menu_item(const std::string &name) const
{
    for (const auto& m : menu)
    {
        if (m.name == name)
        {
            return &m;
        }
    }
    return nullptr;
}

ctrl_command_t::ctrl_command_t(ctrl_id_t id
                               , ctrl_value_t value
                               , bool is_set
                               , bool success
                               , std::uint32_t delay_ms)
    : id(id)
    , value(value)
    , is_set(is_set)
    , success(success)
    , delay_ms(delay_ms)
{

}

buffer_item_t &mapped_buffer_t::current()
{
    return buffers[index];
}

void mapped_buffer_t::next()
{
    index = (index + 1) % buffers.size();
}


}
