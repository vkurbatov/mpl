#include "compose_video_track_params.h"

namespace mpl::media
{

compose_video_track_params_t::compose_video_track_params_t(const std::string_view &name
                                                           , bool enabled
                                                           , const draw_options_t &draw_options
                                                           , double animation
                                                           , const std::string_view &user_image_path
                                                           , timestamp_t timeout)
    : track_params_t(name
                     , enabled)
    , draw_options(draw_options)
    , animation(animation)
    , user_image_path(user_image_path)
    , timeout(timeout)
{

}

bool compose_video_track_params_t::operator ==(const compose_video_track_params_t &other) const
{
    return track_params_t::operator == (other)
            && draw_options == other.draw_options
            && animation == other.animation
            && user_image_path == other.user_image_path
            && timeout == other.timeout;
}

bool compose_video_track_params_t::operator !=(const compose_video_track_params_t &other) const
{
    return ! operator == (other);
}


}
