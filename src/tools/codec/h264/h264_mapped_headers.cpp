#include "h264_mapped_headers.h"

#include <cstring>

namespace pt::codec
{

namespace detail
{

h264_nalu_type_t fetch_nalu_type(std::uint8_t nal)
{
    auto nalu_type = static_cast<h264_nalu_type_t>(nal);
    if (nalu_type > h264_nalu_type_t::unspecified
            && nalu_type < h264_nalu_type_t::nal_30)
    {
        return nalu_type;
    }

    return h264_nalu_type_t::undefined;
}

}

h264_fragmentation_type_t avc_mapped_header_t::fragmentation_type() const
{
    return static_cast<h264_fragmentation_type_t>(length_size_minus_one + 1);
}

bool avc_mapped_header_t::is_valid() const
{
    return conf_version == 0x01
            && reserved_1 == 0x3f
            && reserved_2 == 0x07;
}

h264_nalu_type_t nal_mapped_header_t::nalu_type() const
{
    return detail::fetch_nalu_type(nal);
}

const void *nal_mapped_header_t::payload() const
{
    return reinterpret_cast<const std::uint8_t*>(this) + 1;
}

void *nal_mapped_header_t::payload()
{
    return reinterpret_cast<std::uint8_t*>(this) + 1;
}

h264_nalu_type_t fu_a_header_t::nalu_type() const
{
    return detail::fetch_nalu_type(nal);
}

}
