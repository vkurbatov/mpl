#include "sps_header.h"

#include "tools/utils/check_defs.h"
#include "tools/utils/bitstream_base.h"

namespace pt::codec
{

bool sps_header_t::parse(const void *stream_data
                         , std::size_t stream_size)
{
    pt::utils::bit_stream_reader reader(stream_data
                                             , stream_size * 8
                                             , true);

    std::uint32_t golomb_dummy = 0;
    // profile_idc: u(8)
    __check_empty_on_fail(reader.pop(&profile_idc, 8));

    // constraint_sets: u(8)
    __check_empty_on_fail(reader.pop(&constraints, 8));

    // level_idc: u(8)
    __check_empty_on_fail(reader.pop(&level_idc, 8));

    // seq_parameter_set_id: ue(v)
    __check_empty_on_fail(reader.pop_golomb(id));
    switch(profile_idc)
    {
        case 100:
        case 110:
        case 122:
        case 244:
        case 44:
        case 83:
        case 86:
        case 118:
        case 128:
        case 138:
        case 139:
        case 134:
        {
            // chroma_format_idc: ue(v)
            __check_empty_on_fail(reader.pop_golomb(chroma_format_idc));
            if (chroma_format_idc == 3)
            {
                // separate_colour_plane_flag: u(1)
                __check_empty_on_fail(reader.pop(&separate_colour_plane_flag, 1));
            }

            // bit_depth_luma_minus8: ue(v)
            __check_empty_on_fail(reader.pop_golomb(golomb_dummy));

            // bit_depth_chroma_minus8: ue(v)
            __check_empty_on_fail(reader.pop_golomb(golomb_dummy));

            // qpprime_y_zero_transform_bypass_flag: u(1)
            __check_empty_on_fail(reader.skip(1));

            // seq_scaling_matrix_present_flag: u(1)
            std::uint32_t seq_scaling_matrix_present_flag;
            __check_empty_on_fail(reader.pop(&seq_scaling_matrix_present_flag, 1));

            if (seq_scaling_matrix_present_flag)
            {
                std::int32_t scaling_list_count = (chroma_format_idc == 3 ? 12 : 8);
                for (int i = 0; i < scaling_list_count; i++)
                {
                    // seq_scaling_list_present_flag[i]  : u(1)
                    std::uint32_t seq_scaling_list_present_flags;
                    __check_empty_on_fail(reader.pop(&seq_scaling_list_present_flags, 1));

                    if (seq_scaling_list_present_flags != 0)
                    {
                        std::int32_t last_scale = 8;
                        std::int32_t next_scale = 8;
                        std::int32_t size_of_scaling_list = i < 6 ? 16 : 64;

                        for (int j = 0; j < size_of_scaling_list; j++)
                        {
                            if (next_scale != 0)
                            {
                                // delta_scale: se(v)
                                std::int32_t delta_scale;
                                __check_empty_on_fail(reader.pop_golomb(delta_scale));
                                next_scale = (last_scale + delta_scale + 256) % 256;
                            }
                            if (next_scale != 0)
                            {
                                last_scale = next_scale;
                            }
                        }
                    }
                }
            }

        }
        break;
        default:;
    }

    // log2_max_frame_num_minus4: ue(v)
    std::uint32_t log2_max_frame_num_minus4 = 0;
    __check_empty_on_fail(reader.pop_golomb(log2_max_frame_num_minus4));

    log2_max_frame_num = log2_max_frame_num_minus4 + 4;

    // pic_order_cnt_type: ue(v)
    __check_empty_on_fail(reader.pop_golomb(pic_order_cnt_type) > 0
                          && pic_order_cnt_type <= 28);

    if (pic_order_cnt_type == 0)
    {
        // log2_max_pic_order_cnt_lsb_minus4: ue(v)
        std::uint32_t log2_max_pic_order_cnt_lsb_minus4 = 0;
        __check_empty_on_fail(reader.pop_golomb(log2_max_pic_order_cnt_lsb_minus4) > 0
                              && log2_max_pic_order_cnt_lsb_minus4 <= 28);

        log2_max_pic_order_cnt_lsb = log2_max_pic_order_cnt_lsb_minus4 + 4;
    }
    else if (pic_order_cnt_type == 1)
    {
        // delta_pic_order_always_zero_flag: u(1)
        __check_empty_on_fail(reader.pop(&delta_pic_order_always_zero_flag, 1));

        // offset_for_non_ref_pic: se(v)
        __check_empty_on_fail(reader.pop_golomb(golomb_dummy));

        // offset_for_top_to_bottom_field: se(v)
        __check_empty_on_fail(reader.pop_golomb(golomb_dummy));

        // num_ref_frames_in_pic_order_cnt_cycle: ue(v)
        std::uint32_t num_ref_frames_in_pic_order_cnt_cycle = 0;
        __check_empty_on_fail(reader.pop_golomb(num_ref_frames_in_pic_order_cnt_cycle));

        while(num_ref_frames_in_pic_order_cnt_cycle-- > 0)
        {
            // offset_for_ref_frame[i]: se(v)
            __check_empty_on_fail(reader.pop_golomb(golomb_dummy));
        }
    }

    // max_num_ref_frames: ue(v)
    __check_empty_on_fail(reader.pop_golomb(max_num_ref_frames));

    // gaps_in_frame_num_value_allowed_flag: u(1)
    __check_empty_on_fail(reader.skip(1));

    // pic_width_in_mbs_minus1: ue(v)
    std::uint32_t pic_width_in_mbs_minus1 = 0;
    __check_empty_on_fail(reader.pop_golomb(pic_width_in_mbs_minus1));

    // pic_height_in_map_units_minus1: ue(v)
    std::uint32_t pic_height_in_map_units_minus1 = 0;
    __check_empty_on_fail(reader.pop_golomb(pic_height_in_map_units_minus1));

    // frame_mbs_only_flag: u(1)
    __check_empty_on_fail(reader.pop(&frame_mbs_only_flag, 1));

    if (frame_mbs_only_flag == 0)
    {
        // mb_adaptive_frame_field_flag: u(1)
        __check_empty_on_fail(reader.skip(1));
    }

    // direct_8x8_inference_flag: u(1)
    __check_empty_on_fail(reader.skip(1));

    // frame_cropping_flag: u(1)
    std::uint32_t frame_cropping_flag = 0;
    std::uint32_t frame_crop_left_offset = 0;
    std::uint32_t frame_crop_right_offset = 0;
    std::uint32_t frame_crop_top_offset = 0;
    std::uint32_t frame_crop_bottom_offset = 0;

    __check_empty_on_fail(reader.pop(&frame_cropping_flag, 1));

    if (frame_cropping_flag)
    {
        __check_empty_on_fail(reader.pop_golomb(frame_crop_left_offset));
        __check_empty_on_fail(reader.pop_golomb(frame_crop_right_offset));
        __check_empty_on_fail(reader.pop_golomb(frame_crop_top_offset));
        __check_empty_on_fail(reader.pop_golomb(frame_crop_bottom_offset));
    }

    // vui_parameters_present_flag: u(1)
    __check_empty_on_fail(reader.pop(&vui_params_present, 1));

    width = 16 * (pic_width_in_mbs_minus1 + 1);
    height = 16 * (2 - frame_mbs_only_flag) * (pic_height_in_map_units_minus1 + 1);

    if (separate_colour_plane_flag != 0
            || chroma_format_idc == 0)
    {
        frame_crop_bottom_offset *= (2 - frame_mbs_only_flag);
        frame_crop_top_offset *= (2 - frame_mbs_only_flag);
    }
    else if (separate_colour_plane_flag == 0
             && chroma_format_idc > 0)
    {
        // (4:2:0) and (4:2:2)
        if (chroma_format_idc == 1
                || chroma_format_idc == 2)
        {
            frame_crop_left_offset *= 2;
            frame_crop_right_offset *= 2;
        }

        // (4:2:0)
        if (chroma_format_idc == 1)
        {
            frame_crop_top_offset *= 2;
            frame_crop_bottom_offset *= 2;
        }
    }

    width -= (frame_crop_left_offset + frame_crop_right_offset);
    height -= (frame_crop_top_offset + frame_crop_bottom_offset);

    return true;
}

uint32_t sps_header_t::profile_level() const
{
    return static_cast<std::uint32_t>(profile_idc) << 0
            | static_cast<std::uint32_t>(constraints) << 8
            | static_cast<std::uint32_t>(level_idc) << 16;
}



}
