#include "flow_stats.h"

namespace mpl
{

flow_stats_t::flow_stats_t(std::size_t id
                           , std::size_t total_frames
                           , std::size_t total_bytes
                           , double framerate
                           , std::size_t bitrate)
    : id(id)
    , total_frames(total_frames)
    , total_bytes(total_bytes)
    , framerate(framerate)
    , bitrate(bitrate)
{

}

bool flow_stats_t::operator ==(const flow_stats_t &other) const
{
    return id == other.id
            && total_frames == other.total_frames
            && total_bytes == other.total_bytes
            && framerate == other.framerate
            && bitrate == other.bitrate;
}

bool flow_stats_t::operator !=(const flow_stats_t &other) const
{
    return ! operator == (other);
}

void flow_stats_t::push_frame(std::size_t frame_size)
{
    total_frames++;
    total_bytes += frame_size;
}

void flow_stats_t::reset()
{
    *this = {};

}

}
