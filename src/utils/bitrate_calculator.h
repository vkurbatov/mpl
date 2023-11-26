#ifndef MPL_BITRATE_CALCULATOR_H
#define MPL_BITRATE_CALCULATOR_H

#include "core/time_types.h"
#include "core/flow_stats.h"

namespace mpl::utils
{

class bitrate_calculator
{
public:

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
