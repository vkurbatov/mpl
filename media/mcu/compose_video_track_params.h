#ifndef MPL_MEDIA_COMPOSE_VIDEO_TRACK_PARAMS_H
#define MPL_MEDIA_COMPOSE_VIDEO_TRACK_PARAMS_H

#include "core/time_types.h"
#include "media/track_params.h"
#include "media/draw_options.h"

namespace mpl::media
{

struct compose_video_track_params_t : public track_params_t
{
    draw_options_t          draw_options;
    double                  animation;
    std::string             user_image_path;
    timestamp_t             timeout;
public:
    compose_video_track_params_t(const std::string_view& name = {}
            , bool enabled = false
            , const draw_options_t& draw_options = {}
            , double animation = 0.0
            , const std::string_view& user_image_path = {}
            , timestamp_t timeout = timestamp_null);

    bool operator == (const compose_video_track_params_t& other) const;
    bool operator != (const compose_video_track_params_t& other) const;
};

}

#endif // MPL_MEDIA_COMPOSE_VIDEO_TRACK_PARAMS_H
