#ifndef MPL_VIDEO_TYPES_H
#define MPL_VIDEO_TYPES_H

namespace mpl::media
{

enum class video_format_id_t
{
    undefined = 0,
    yuv420p,
    yuv422p,
    yuv444p,
    yuv411p,
    yuyv,
    uyvy,
    yuv410,
    nv12,
    nv21,
    nv16,
    bgr555,
    bgr555x,
    bgr565,
    bgr565x,
    rgb555,
    rgb555x,
    rgb565,
    rgb565x,
    bgr8,
    rgb8,
    bgr24,
    rgb24,
    bgr32,
    rgb32,
    abgr32,
    argb32,
    bgra32,
    rgba32,
    gray8,
    gray16,
    gray16x,
    sbggr8,
    sgbrg8,
    sgrbg8,
    srggb8,
    png,
    jpeg,
    mjpeg,
    gif,
    h265,
    h264,
    h263,
    h263p,
    h261,
    vp8,
    vp9,
    mpeg4,
    cpia
};

}

#endif // MPL_VIDEO_TYPES_H
