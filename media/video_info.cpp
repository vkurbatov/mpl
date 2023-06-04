#include "video_info.h"

#include "format_utils.h"

#include <string>
#include <unordered_map>

namespace mpl::media
{

namespace detail
{

const video_format_info_t video_format_info_table[] =
{
    //bpp enc     planar   conv,   motion ch, aw ah fourcc
    { -1, false,  false,   false,  false, 0, 1, 1, utils::to_fourcc({})       },    // undefined
    { 12, false,  true,    true,   false, 3, 2, 2, utils::to_fourcc("YU12")   },    // yuv420p
    { 16, false,  true,    true,   false, 3, 2, 1, utils::to_fourcc("422P")   },    // yuv422p
    { 24, false,  true,    true,   false, 3, 1, 1, utils::to_fourcc("Y444")   },    // yuv444p
    { 12, false,  true,    true,   false, 3, 4, 1, utils::to_fourcc("411P")   },    // yuv411p
    { 12, false,  false,   true,   false, 3, 2, 1, utils::to_fourcc("YUYV")   },    // yuyv
    { 12, false,  false,   true,   false, 3, 2, 1, utils::to_fourcc("UYVY")   },    // uyvy
    { 9,  false,  false,   true,   false, 3, 4, 4, utils::to_fourcc("YUV9")   },    // yuv410
    { 12, false,  true,    true,   false, 3, 2, 1, utils::to_fourcc("NV12")   },    // nv12
    { 12, false,  true,    true,   false, 3, 2, 0, utils::to_fourcc("NV21")   },    // nv21
    { 16, false,  false,   true,   false, 3, 2, 2, utils::to_fourcc("NV16")   },    // nv16
    { 16, false,  false,   true,   false, 3, 1, 1, utils::to_fourcc("BGRO")   },    // bgr555
    { 16, false,  false,   true,   false, 3, 1, 1, utils::to_fourcc("BGRQ")   },    // bgr555x
    { 16, false,  false,   true,   false, 3, 1, 1, utils::to_fourcc("BGRP")   },    // bgr565
    { 16, false,  false,   true,   false, 3, 1, 1, utils::to_fourcc("BGRR")   },    // bgr565x
    { 16, false,  false,   true,   false, 3, 1, 1, utils::to_fourcc("RGBO")   },    // rgb555
    { 16, false,  false,   true,   false, 3, 1, 1, utils::to_fourcc("RGBQ")   },    // rgb555x
    { 16, false,  false,   true,   false, 3, 1, 1, utils::to_fourcc("RGBP")   },    // rgb565
    { 16, false,  false,   true,   false, 3, 1, 1, utils::to_fourcc("RGBR")   },    // rgb565x
    { 8,  false,  false,   true,   false, 3, 1, 1, utils::to_fourcc("BGR1")   },    // bgr8
    { 8,  false,  false,   true,   false, 3, 1, 1, utils::to_fourcc("RGB1")   },    // rgb8
    { 24, false,  false,   true,   false, 3, 1, 1, utils::to_fourcc("BGR3")   },    // bgr24
    { 24, false,  false,   true,   false, 3, 1, 1, utils::to_fourcc("RGB3")   },    // rgb24
    { 32, false,  false,   true,   false, 3, 1, 1, utils::to_fourcc("BGR4")   },    // bgr32
    { 32, false,  false,   true,   false, 3, 1, 1, utils::to_fourcc("RGB4")   },    // rgb32
    { 32, false,  false,   true,   false, 4, 1, 1, utils::to_fourcc("AR24")   },    // abgr32
    { 32, false,  false,   true,   false, 4, 1, 1, utils::to_fourcc("BA24")   },    // argb32
    { 32, false,  false,   true,   false, 4, 1, 1, utils::to_fourcc("RA24")   },    // bgra32
    { 32, false,  false,   true,   false, 4, 1, 1, utils::to_fourcc("AB24")   },    // rgba32
    { 8,  false,  false,   true,   false, 1, 1, 1, utils::to_fourcc("GREY")   },    // gray8
    { 16, false,  false,   true,   false, 1, 1, 1, utils::to_fourcc("Y16 ")   },    // gray16
    { 16, false,  false,   true,   false, 1, 1, 1, utils::to_fourcc("Y16 ", true) },// gray16x
    { 8,  false,  false,   true,   false, 4, 1, 1, utils::to_fourcc("BA81")   },    // sbggr8
    { 8,  false,  false,   true,   false, 4, 1, 1, utils::to_fourcc("GBRG")   },    // sgbrg8
    { 8,  false,  false,   true,   false, 4, 1, 1, utils::to_fourcc("GRBG")   },    // sgrbg8
    { 8,  false,  false,   true,   false, 4, 1, 1, utils::to_fourcc("RGGB")   },    // srggb8
    { 0,  true,   false,   false,  false, 0, 1, 1, utils::to_fourcc("PNG ")   },    // png
    { 0,  true,   false,   false,  false, 0, 1, 1, utils::to_fourcc("JPEG")   },    // jpeg
    { 0,  true,   false,   false,  false, 0, 1, 1, utils::to_fourcc("MJPG")   },    // mjpeg
    { 0,  true,   false,   false,  false, 0, 1, 1, utils::to_fourcc("GIF ")   },    // gif
    { 0,  true,   false,   false,  true,  0, 1, 1, utils::to_fourcc("HEVC")   },    // h265
    { 0,  true,   false,   false,  true,  0, 1, 1, utils::to_fourcc("H264")   },    // h264
    { 0,  true,   false,   false,  true,  0, 1, 1, utils::to_fourcc("H263")   },    // h263
    { 0,  true,   false,   false,  true,  0, 1, 1, utils::to_fourcc("H263")   },    // h263p
    { 0,  true,   false,   false,  true,  0, 1, 1, utils::to_fourcc("H261")   },    // h261
    { 0,  true,   false,   false,  true,  0, 1, 1, utils::to_fourcc("VP80")   },    // vp8
    { 0,  true,   false,   false,  true,  0, 1, 1, utils::to_fourcc("VP90")   },    // vp9
    { 0,  true,   false,   false,  true,  0, 1, 1, utils::to_fourcc("MPG4")   },    // mpeg4,
    { 0,  true,   false,   false,  true,  0, 1, 1, utils::to_fourcc("CPIA")   },    // cpia
};

using reverse_map_t = std::unordered_multimap<std::uint32_t, video_format_id_t>;

reverse_map_t get_reverse_table()
{
    reverse_map_t result;
    std::int32_t format_idx = -1;

    for (const auto& info : video_format_info_table)
    {
        if (info.fourcc != 0)
        {
            result.emplace(info.fourcc
                           , static_cast<video_format_id_t>(format_idx));
        }

        format_idx++;
    }

    return result;
}

}

const video_format_info_t &video_format_info_t::get_info(video_format_id_t format_id)
{
    return detail::video_format_info_table[static_cast<std::int32_t>(format_id) + 1];
}

video_format_id_t video_format_info_t::format_from_fourcc(uint32_t fourcc)
{
    static const auto format_table = detail::get_reverse_table();
    if (auto it = format_table.find(fourcc); it != format_table.end())
    {
        return it->second;
    }

    return video_format_id_t::undefined;
}


}
