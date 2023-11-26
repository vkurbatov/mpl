#ifndef MPL_MEDIA_TIMESTAMP_CALCULATOR_H
#define MPL_MEDIA_TIMESTAMP_CALCULATOR_H

#include "core/time_types.h"

namespace mpl::media
{

class timestamp_calculator
{
    std::uint32_t   m_sample_rate;
    timestamp_t     m_start_time;
    timestamp_t     m_timestamp;

public:

    timestamp_calculator(std::uint32_t sample_rate);

    timestamp_t calc_timestamp(std::uint32_t frame_time);
    void reset(std::uint32_t sample_rate);
    void reset();

    timestamp_t elapsed() const;
};

}

#endif // MPL_MEDIA_TIMESTAMP_CALCULATOR_H
