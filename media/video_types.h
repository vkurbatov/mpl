#ifndef MPL_VIDEO_TYPES_H
#define MPL_VIDEO_TYPES_H

namespace mpl
{

enum class video_format_id_t
{
    undefined = -1,
    rgb,
    rgba,
    argb,
    bgr,
    bgra,
    abgr,
    gray8,
    nv12,
    yuv420p,
    yuv422p,
    jpeg,
    gif,
    png,
    h263,
    h263p,
    h264,
    h265,
    vp8,
    vp9
};

}

#endif // MPL_VIDEO_TYPES_H
