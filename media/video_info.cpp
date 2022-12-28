#include "video_info.h"

namespace mpl
{

namespace detail
{

const video_format_info_t video_format_info_table[] =
{
    //bpp enc     planar  conv,  motion aw ah
    { -1, false,  false,  false, false, 1, 1 },  // undefined
    { 24, false,  false,  true,  false, 1, 1 },  // rgb
    { 32, false,  false,  true,  false, 1, 1},   // rgba
    { 32, false,  false,  true,  false, 1, 1 },  // argb
    { 24, false,  false,  true,  false, 1, 1 },  // bgr
    { 32, false,  false,  true,  false, 1, 1 },  // bgra
    { 32, false,  false,  true,  false, 1, 1 },  // abrg
    { 8,  false,  false,  true,  false, 1, 1 },  // gray8
    { 12, false,  true,   true,  false, 2, 1 },  // nv12
    { 12, false,  true,   true,  false, 2, 1 },  // yuv420p
    { 16, false,  true,   true,  false, 2, 2 },  // yuv422p
    { 0,  true,   false,  false, false, 1, 1 },  // jpeg
    { 0,  true,   false,  false, false, 1, 1 },  // gif
    { 0,  true,   false,  false, false, 1, 1 },  // png
    { 0,  true,   false,  false, true,  1, 1 },  // h263
    { 0,  true,   false,  false, true,  1, 1 },  // h263p
    { 0,  true,   false,  false, true,  1, 1 },  // h264
    { 0,  true,   false,  false, true,  1, 1 },  // h265
    { 0,  true,   false,  false, true,  1, 1 },  // vp8
    { 0,  true,   false,  false, true,  1, 1 },  // vp9
};

}

const video_format_info_t &video_format_info_t::get_info(video_format_id_t format_id)
{
    return detail::video_format_info_table[static_cast<std::int32_t>(format_id)];
}

}
