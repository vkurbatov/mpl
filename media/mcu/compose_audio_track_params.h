#ifndef MPL_MEDIA_COMPOSE_AUDIO_TRACK_PARAMS_H
#define MPL_MEDIA_COMPOSE_AUDIO_TRACK_PARAMS_H

#include "media/track_params.h"

namespace mpl::media
{

struct compose_audio_track_params_t : public track_params_t
{
    double      volume;

    compose_audio_track_params_t(const std::string_view& name = {}
                                , bool enabled = false
                                , double volume = 0.0);

    bool operator == (const compose_audio_track_params_t& other) const;
    bool operator != (const compose_audio_track_params_t& other) const;
};

}

#endif // MPL_MEDIA_COMPOSE_AUDIO_TRACK_PARAMS_H
