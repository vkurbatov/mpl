#ifndef MPL_I_MEDIA_TRANSCODER_FACTORY_H
#define MPL_I_MEDIA_TRANSCODER_FACTORY_H

#include "i_media_transcoder.h"

namespace mpl::media
{

class i_media_transcoder_factory
{
public:
    using u_ptr_t = std::unique_ptr<i_media_transcoder_factory>;
    using s_ptr_t = std::shared_ptr<i_media_transcoder_factory>;

    virtual ~i_media_transcoder_factory() = default;
    virtual i_media_transcoder::u_ptr_t create_transcoder(const i_property& property) = 0;
};

}

#endif // MPL_I_MEDIA_TRANSCODER_FACTORY_H
