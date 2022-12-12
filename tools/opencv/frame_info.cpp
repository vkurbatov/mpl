#include "frame_info.h"
#include "ocv_utils.h"

namespace ocv
{

frame_info_t::frame_info_t(const frame_format_t &format
                           , const frame_size_t &size)
    : format(format)
    , size(size)
{

}

std::size_t frame_info_t::frame_size() const
{
    return utils::get_format_info(format).bits_per_second * size.size() / vars::color_depth;
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
