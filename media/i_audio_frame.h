#ifndef MPL_I_AUDIO_FRAME_H
#define MPL_I_AUDIO_FRAME_H

#include "i_media_frame.h"

namespace mpl
{

class i_audio_format;

class i_audio_frame : public i_media_frame
{
public:
    using u_ptr_t = std::unique_ptr<i_audio_frame>;
    using s_ptr_t = std::shared_ptr<i_audio_frame>;

    virtual ~i_audio_frame() = default;
    virtual const i_audio_format& format() const = 0;
};

}

#endif // MPL_I_AUDIO_FRAME_H
