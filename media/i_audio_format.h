#ifndef MPL_I_AUDIO_FORMAT_H
#define MPL_I_AUDIO_FORMAT_H

#include "audio_types.h"
#include "i_media_format.h"
#include <cstdint>

namespace mpl
{

class i_audio_format : public i_media_format
{
public:
    using u_ptr_t = std::unique_ptr<i_audio_format>;
    using s_ptr_t = std::shared_ptr<i_audio_format>;

    virtual audio_format_id_t format_id() const = 0;
    virtual std::int32_t sample_rate() const = 0;
    virtual std::int32_t channels() const = 0;
};

}

#endif // MPL_I_AUDIO_FORMAT_H
