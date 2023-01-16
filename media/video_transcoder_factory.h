#ifndef MPL_VIDEO_TRANSCODER_FACTORY_H
#define MPL_VIDEO_TRANSCODER_FACTORY_H

#include "i_media_transcoder_factory.h"

namespace mpl::media
{

class video_transcoder_factory : public i_media_transcoder_factory
{
public:
    video_transcoder_factory();

    // i_media_transcoder_factory interface
public:
    i_media_transcoder::u_ptr_t create_transcoder(const i_property &property) override;
};

}

#endif // MPL_VIDEO_TRANSCODER_FACTORY_H
