#ifndef MPL_I_MEDIA_TRANSCODER_H
#define MPL_I_MEDIA_TRANSCODER_H

#include "i_media_format.h"
#include "i_message_channel.h"

namespace mpl
{

class i_media_transcoder : public i_message_channel
{
public:
    using u_ptr_t = std::unique_ptr<i_media_transcoder>;
    using s_ptr_t = std::shared_ptr<i_media_transcoder>;

    virtual const i_media_format& input_format() const = 0;
    virtual const i_media_format& output_format() const = 0;
};

}

#endif // MPL_I_MEDIA_TRANSCODER_H
