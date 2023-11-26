#ifndef MPL_FLOW_STATS_H
#define MPL_FLOW_STATS_H

#include <cstring>

namespace mpl
{

struct flow_stats_t
{
    std::size_t     id;
    std::size_t     total_frames;
    std::size_t     total_bytes;
    double          framerate;
    std::size_t     bitrate;

    flow_stats_t(std::size_t id = 0
                 , std::size_t total_frames = 0
                 , std::size_t total_bytes = 0
                 , double framerate = 0.0
                 , std::size_t bitrate = 0);

    bool operator == (const flow_stats_t& other) const;
    bool operator != (const flow_stats_t& other) const;

    void push_frame(std::size_t frame_size);


    void reset();

};

}

#endif // MPL_FLOW_STATS_H
