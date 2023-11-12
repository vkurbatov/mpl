#ifndef MPL_I_VIDEO_FRAME_H
#define MPL_I_VIDEO_FRAME_H

#include "i_media_frame.h"

namespace mpl::media
{

class i_video_format;

class i_video_frame : public i_media_frame
{
public:

    using u_ptr_t = std::unique_ptr<i_video_frame>;
    using s_ptr_t = std::shared_ptr<i_video_frame>;

    virtual ~i_video_frame() = default;
    virtual const i_video_format& format() const = 0;
    virtual video_frame_type_t frame_type() const = 0;
};

}

#endif // MPL_I_VIDEO_FRAME_H
