#include "h264_fragment.h"

namespace pt::codec
{

h264_fragment_t::h264_fragment_t(std::size_t offset
                                 , std::size_t payload_offset
                                 , std::size_t payload_length)
    : offset(offset)
    , payload_offset(payload_offset)
    , payload_length(payload_length)
{

}

std::size_t h264_fragment_t::header_size() const
{
    return payload_offset - offset;
}

std::size_t h264_fragment_t::total_size() const
{
    return header_size() + payload_length;
}


}
