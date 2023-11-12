#ifndef PT_CODEC_H264_SPS_HEADER_H
#define PT_CODEC_H264_SPS_HEADER_H

#include <cstdint>

namespace pt::codec
{

struct sps_header_t
{
    std::uint32_t profile_idc = 0;
    std::uint32_t constraints = 0;
    std::uint32_t level_idc = 0;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::uint32_t chroma_format_idc = 0;
    std::uint32_t delta_pic_order_always_zero_flag = 0;
    std::uint32_t separate_colour_plane_flag = 0;
    std::uint32_t frame_mbs_only_flag = 0;
    std::uint32_t log2_max_frame_num = 4;
    std::uint32_t log2_max_pic_order_cnt_lsb = 4;
    std::uint32_t pic_order_cnt_type = 0;
    std::uint32_t max_num_ref_frames = 0;
    std::uint32_t vui_params_present = 0;
    std::uint32_t id = 0;

    bool parse(const void* stream_data
               , std::size_t stream_size);

    std::uint32_t profile_level() const;

};

}

#endif // PT_CODEC_H264_SPS_HEADER_H
