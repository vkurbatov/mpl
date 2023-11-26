#include "compose_stream_params.h"

namespace mpl::media
{

compose_stream_params_t::compose_stream_params_t(int32_t order
                                                 , const std::string_view &name
                                                 , const compose_audio_track_params_t &audio_track
                                                 , const compose_video_track_params_t &video_track)
    : order(order)
    , name(name)
    , audio_track(audio_track)
    , video_track(video_track)
{

}

bool compose_stream_params_t::operator ==(const compose_stream_params_t &other) const
{
    return order == other.order
            && name == other.name
            && audio_track == other.audio_track
            && video_track == other.video_track;
}

bool compose_stream_params_t::operator !=(const compose_stream_params_t &other) const
{
    return ! operator == (other);
}



}
