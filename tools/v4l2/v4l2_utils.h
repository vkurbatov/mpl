#ifndef V4L2_UTILS_H
#define V4L2_UTILS_H

#include <string>
#include <vector>

namespace v4l2
{

std::string get_ctrl_name(std::uint32_t ctrl_id);
std::uint32_t get_ctrl_id(const std::string_view& ctrl_name);

std::string get_format_name(std::uint32_t format_id);
std::uint32_t get_format_id(const std::string_view& format_name);

}

#endif // V4L2_UTILS_H
