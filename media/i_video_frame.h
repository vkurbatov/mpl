#ifndef MPL_I_VIDEO_FRAME_H
#define MPL_I_VIDEO_FRAME_H

#include "i_media_frame.h"

namespace mpl
{

class i_video_format;

class i_audio_frame : public i_media_frame
{
public:

    enum class frame_type_t
    {
        undefined = -1,
        delta_frame,
        key_frame,
        image_frame
    };

    using u_ptr_t = std::unique_ptr<i_audio_frame>;
    using s_ptr_t = std::shared_ptr<i_audio_frame>;

    virtual ~i_audio_frame() = default;
    virtual const i_video_format& format() const = 0;
    virtual frame_type_t frame_type() const = 0;
};

#endif // MPL_I_VIDEO_FRAME_H
