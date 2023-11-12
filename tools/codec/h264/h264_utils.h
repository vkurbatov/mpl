#ifndef PT_CODEC_H264_UTILS_H
#define PT_CODEC_H264_UTILS_H

#include "h264_types.h"
#include "h264_fragment.h"

namespace pt::codec
{

h264_fragmentation_type_t get_fragmentation_type(const void* data
                                                 , std::size_t size);
h264_fragment_t::array_t split_fragments(h264_fragmentation_type_t type
                                         , const void* data
                                         , std::size_t size);


}

#endif // PT_CODEC_H264_UTILS_H
