#ifndef PT_CODEC_H264_TYPES_H
#define PT_CODEC_H264_TYPES_H

#include <cstdint>

namespace pt::codec
{

enum class h264_nalu_type_t
{
    undefined       = -1,
    unspecified     = 0,
    non_idr_slice   = 1,
    dp_a_slice      = 2,
    dp_b_slice      = 3,
    dp_c_slice      = 4,
    idr_slice       = 5,
    sei             = 6,
    sps             = 7,
    pps             = 8,
    access_unit     = 9,
    end_of_seq      = 10,
    end_of_stream   = 11,
    filler_data     = 12,
    seq_ex          = 13,
    prefix          = 14,
    subset_sps      = 15,
    dps             = 16,
    nal_17          = 17,
    nal_18          = 18,
    slice_layer     = 19,
    slice_layer_ex  = 20,
    slice_layer_ex2 = 21,
    nal_22          = 22,
    nal_23          = 23,
    stap_a          = 24,
    stap_b          = 25,
    mtap_16         = 26,
    mtap_24         = 27,
    fu_a            = 28,
    fu_b            = 29,
    nal_30          = 30,
    nal_31          = 31,
};

enum class h264_fragmentation_type_t
{
    undefined       = 0,
    avcc_8          = 1,
    avcc_16         = 2,
    avcc_24         = 3,
    avcc_32         = 4,
    annex_b         = 5
};

enum class h264_packetization_mode_t
{
    single_nal      = 0,
    non_interleaved = 1,
    interleaved     = 2
};

enum class h264_format_parameter_id_t
{
    undefined = -1,
    profile_level_id = 0,
    level_asymmetry_allowed = 1,
    packetization_mode = 2
};

static constexpr std::uint32_t annex_b_start_code = 0x01000000;
static constexpr std::size_t annex_b_min_header_size = 3;
static constexpr std::size_t annex_b_min_frame_size = annex_b_min_header_size + 1;

}

#endif // PT_CODEC_H264_TYPES_H
