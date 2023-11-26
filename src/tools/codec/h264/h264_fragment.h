#ifndef PT_CODEC_H264_FRAGMENT_H
#define PT_CODEC_H264_FRAGMENT_H

#include <cstdint>
#include <vector>

namespace pt::codec
{

struct h264_fragment_t
{
    using array_t = std::vector<h264_fragment_t>;

    std::size_t offset;
    std::size_t payload_offset;
    std::size_t payload_length;

    h264_fragment_t(std::size_t offset = 0
                    , std::size_t payload_offset = 0
                    , std::size_t payload_length = 0);

    std::size_t header_size() const;
    std::size_t total_size() const;

};

}

#endif // PT_CODEC_H264_FRAGMENT_H
