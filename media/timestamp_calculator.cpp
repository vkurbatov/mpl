#include "timestamp_calculator.h"
#include "core/time_utils.h"

namespace mpl::media
{

timestamp_calculator::timestamp_calculator(uint32_t sample_rate)
    : m_sample_rate(sample_rate)
{
    reset();
}

timestamp_t timestamp_calculator::calc_timestamp(std::uint32_t frame_time)
{
    if (m_sample_rate > 0
            && frame_time > 0)
    {
        auto now = core::utils::get_ticks();
        auto e_time = ((now - m_start_time) * m_sample_rate) / durations::seconds(1);

        auto new_timestamp = m_timestamp + frame_time;

        auto jitter = new_timestamp - e_time;

        auto max_jitter = m_sample_rate / 2;

        if (jitter > max_jitter)
        {
            // nothig
        }
        else if (jitter < -max_jitter)
        {
            m_timestamp = new_timestamp + max_jitter;
        }
        else
        {
            m_timestamp = new_timestamp;
        }

        return m_timestamp;
    }

    return timestamp_infinite;
}

void timestamp_calculator::reset(uint32_t sample_rate)
{
    m_sample_rate = sample_rate;
    reset();
}

void timestamp_calculator::reset()
{
    m_start_time = core::utils::get_ticks();
}

timestamp_t timestamp_calculator::elapsed() const
{
    return core::utils::get_ticks() - m_start_time;
}


}
