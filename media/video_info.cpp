#include "video_info.h"
#include <string>

namespace mpl
{

namespace detail
{

std::uint32_t to_fourcc(const std::string_view& format)
{
    if (format.size() == sizeof(std::uint32_t))
    {
        return static_cast<std::uint32_t>(format[0]) << 24
               | static_cast<std::uint32_t>(format[1]) << 16
               | static_cast<std::uint32_t>(format[2]) << 8
               | static_cast<std::uint32_t>(format[3]) << 0;
    }

    return 0;
}

const video_format_info_t video_format_info_table[] =
{
    //bpp enc     planar   conv,   motion aw ah
    { -1, false,  false,   false,  false, 1, 1, to_fourcc({})       },    // undefined
    { 12, false,  true,    true,   false, 2, 1, to_fourcc("YU12")   },    // yuv420p
    { 16, false,  true,    true,   false, 2, 2, to_fourcc("422P")   },    // yuv422p
    { 24, false,  true,    true,   false, 1, 1, to_fourcc("Y444")   },    // yuv444p
    { 12, false,  true,    true,   false, 1, 2, to_fourcc("411P")   },    // yuv411p
    { 12, false,  false,   true,   false, 2, 1, to_fourcc("YUYV")   },    // yuyv
    { 12, false,  false,   true,   false, 2, 1, to_fourcc("UYVY")   },    // uyvy
    { 9,  false,  false,   true,   false, 2, 2, to_fourcc("YUV9")   },    // yuv410
    { 12, false,  false,   true,   false, 2, 1, to_fourcc("NV12")   },    // nv12
    { 12, false,  false,   true,   false, 1, 2, to_fourcc("NV21")   },    // nv21
    { 16, false,  false,   true,   false, 2, 2, to_fourcc("NV16")   },    // nv16
    { 16, false,  false,   true,   false, 1, 1 },    // bgr555
    { 16, false,  false,   true,   false, 1, 1 },    // bgr555x
    { 16, false,  false,   true,   false, 1, 1 },    // bgr565
    { 16, false,  false,   true,   false, 1, 1 },    // bgr565x
    { 16, false,  false,   true,   false, 1, 1 },    // rgb555
    { 16, false,  false,   true,   false, 1, 1 },    // rgb555x
    { 16, false,  false,   true,   false, 1, 1 },    // rgb565
    { 16, false,  false,   true,   false, 1, 1 },    // rgb565x
    { 8,  false,  false,   true,   false, 1, 1 },    // bgr8
    { 8,  false,  false,   true,   false, 1, 1 },    // rgb8
    { 24, false,  false,   true,   false, 1, 1 },    // bgr24
    { 24, false,  false,   true,   false, 1, 1 },    // rgb24
    { 32, false,  false,   true,   false, 1, 1 },    // bgr32
    { 32, false,  false,   true,   false, 1, 1 },    // rgb32
    { 32, false,  false,   true,   false, 1, 1 },    // abgr32
    { 32, false,  false,   true,   false, 1, 1 },    // argb32
    { 32, false,  false,   true,   false, 1, 1 },    // bgra32
    { 32, false,  false,   true,   false, 1, 1 },    // rgba32
    { 8,  false,  false,   true,   false, 1, 1 },    // gray8
    { 16, false,  false,   true,   false, 1, 1 },    // gray16
    { 16, false,  false,   true,   false, 1, 1 },    // gray16x
    { 8,  false,  false,   true,   false, 1, 1 },    // sbggr8
    { 8,  false,  false,   true,   false, 1, 1 },    // sgbrg8
    { 8,  false,  false,   true,   false, 1, 1 },    // sgrbg8
    { 8,  false,  false,   true,   false, 1, 1 },    // srggb8
    { 0,  true,   false,   false,  false, 1, 1 },    // jpeg
    { 0,  true,   false,   false,  false, 1, 1 },    // mjpeg
    { 0,  true,   false,   false,  false, 1, 1 },    // gif
    { 0,  true,   false,   false,  true,  1, 1 },    // h265
    { 0,  true,   false,   false,  true,  1, 1 },    // h264
    { 0,  true,   false,   false,  true,  1, 1 },    // h263
    { 0,  true,   false,   false,  true,  1, 1 },    // h263p
    { 0,  true,   false,   false,  true,  1, 1 },    // h261
    { 0,  true,   false,   false,  true,  1, 1 },    // vp8
    { 0,  true,   false,   false,  true,  1, 1 },    // vp9
    { 0,  true,   false,   false,  true,  1, 1 },    // mpeg4,
    { 0,  true,   false,   false,  true,  1, 1 },    // cpia
};

}

const video_format_info_t &video_format_info_t::get_info(video_format_id_t format_id)
{
    return detail::video_format_info_table[static_cast<std::int32_t>(format_id)];
}

}
