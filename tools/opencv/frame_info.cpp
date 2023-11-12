#include "frame_info.h"
#include "ocv_utils.h"

namespace pt::ocv
{

frame_info_t::frame_info_t(const frame_format_t &format
                           , const frame_size_t &size)
    : format(format)
    , size(size)
{

}

bool frame_info_t::operator ==(const frame_info_t &other) const
{
    return format == other.format
            && size == other.size;
}

bool frame_info_t::operator !=(const frame_info_t &other) const
{
    return ! operator == (other);
}

std::size_t frame_info_t::frame_size() const
{
    return utils::get_format_info(format).bps * size.size() / vars::color_depth;
}

const std::string &frame_info_t::format_name() const
{
    return utils::get_format_info(format).name;
}

bool frame_info_t::is_valid() const
{
    return format != frame_format_t::undefined
            && !size.is_null();
}

}
