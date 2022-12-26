#ifndef MPL_AUDIO_INFO_H
#define MPL_AUDIO_INFO_H

#include "audio_types.h"
#include <cstdint>

namespace mpl
{

struct audio_format_info_t
{
    static const audio_format_info_t& get_info(audio_format_id_t format_id);

    std::int32_t    bps;
    std::int32_t    frame_size; // in samples
    bool            encoded;
    bool            planar;
    bool            convertable;
};

}

#endif // MPL_AUDIO_INFO_H
