#ifndef MPL_I_VIDEO_FORMAT_H
#define MPL_I_VIDEO_FORMAT_H

#include "video_types.h"
#include "i_media_format.h"

namespace mpl::media
{

class i_video_format : public i_media_format
{
public:
    using u_ptr_t = std::unique_ptr<i_video_format>;
    using s_ptr_t = std::shared_ptr<i_video_format>;

    virtual video_format_id_t format_id() const = 0;
    virtual std::int32_t width() const = 0;
    virtual std::int32_t height() const = 0;
    virtual double frame_rate() const = 0;
};

}

#endif // MPL_I_VIDEO_FORMAT_H
