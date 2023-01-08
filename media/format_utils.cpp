#include "format_utils.h"

namespace mpl::utils
{

std::uint32_t to_fourcc(const std::string_view& format, bool be)
{
    if (format.size() == sizeof(std::uint32_t))
    {
        if (!be)
        {
            return *reinterpret_cast<const std::uint32_t*>(format.data());
        }
        else
        {
            return static_cast<std::uint32_t>(format[0]) << 24
                   | static_cast<std::uint32_t>(format[1]) << 16
                   | static_cast<std::uint32_t>(format[2]) << 8
                   | static_cast<std::uint32_t>(format[3]) << 0;
        }
    }

    return 0;
}

}

