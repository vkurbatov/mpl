#ifndef MPL_MEDIA_COMPOSE_STREAM_PARAMS_H
#define MPL_MEDIA_COMPOSE_STREAM_PARAMS_H

#include "compose_audio_track_params.h"
#include "compose_video_track_params.h"

namespace mpl::media
{

struct compose_stream_params_t
{
    std::int32_t                    order;
    std::string                     name;
    compose_audio_track_params_t    audio_track;
    compose_video_track_params_t    video_track;

    compose_stream_params_t(std::int32_t order = 0
            , const std::string_view& name = {}
            , const compose_audio_track_params_t& audio_track = {}
            , const compose_video_track_params_t& video_track = {});

    bool operator == (const compose_stream_params_t& other) const;
    bool operator != (const compose_stream_params_t& other) const;
};

}

#endif // MPL_MEDIA_COMPOSE_STREAM_PARAMS_H
