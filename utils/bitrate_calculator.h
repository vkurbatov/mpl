#ifndef MPL_BITRATE_CALCULATOR_H
#define MPL_BITRATE_CALCULATOR_H

#include "core/time_types.h"

namespace mpl
{

class bitrate_calculator
{
public:
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

    static constexpr double br_coeff = 0.5;
    static constexpr timestamp_t calc_interval = durations::seconds(1);


private:

    timestamp_t     m_last_timestamp;

    flow_stats_t    m_current_stats;
    flow_stats_t    m_prev_stats;
public:

    bitrate_calculator();

    // true - if update bitrate info
    bool push_frame(std::size_t frame_size);

    const flow_stats_t& stats() const;

    void reset();
};

}

#endif // MPL_BITRATE_CALCULATOR_H
