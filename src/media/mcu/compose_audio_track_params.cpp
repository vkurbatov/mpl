#include "compose_audio_track_params.h"

namespace mpl::media
{

compose_audio_track_params_t::compose_audio_track_params_t(const std::string_view &name
                                                           , bool enabled
                                                           , double volume)
    : track_params_t(name
                     , enabled)
    , volume(volume)
{

}

bool compose_audio_track_params_t::operator ==(const compose_audio_track_params_t &other) const
{
    return track_params_t::operator == (other)
            && volume == other.volume;
}

bool compose_audio_track_params_t::operator !=(const compose_audio_track_params_t &other) const
{
    return ! operator == (other);
}

}
