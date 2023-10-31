#include "bitrate_calculator.h"

#include "time_utils.h"

namespace mpl::utils
{

bitrate_calculator::bitrate_calculator()
{
    reset();
}


bool bitrate_calculator::push_frame(std::size_t frame_size)
{
    m_current_stats.push_frame(frame_size);

    auto dt = utils::time::get_ticks() - m_last_timestamp;

    if (dt >= calc_interval)
    {
        auto k =  static_cast<double>(dt) / static_cast<double>(calc_interval);

        auto framerate = (m_current_stats.total_frames - m_prev_stats.total_frames) * k;
        auto bitrate = (m_current_stats.total_bytes - m_prev_stats.total_bytes) * 8 * k;

        if (m_prev_stats.framerate == 0)
        {
            m_current_stats.framerate = framerate;
            m_current_stats.bitrate = bitrate;
        }
        else
        {
            m_current_stats.framerate += (framerate - m_current_stats.framerate) * br_coeff;
            m_current_stats.bitrate += (bitrate - m_current_stats.bitrate) * br_coeff;
        }

        m_current_stats.id ++;
        m_last_timestamp += dt;

        m_prev_stats = m_current_stats;

        return true;
    }

    return false;
}

const flow_stats_t &bitrate_calculator::stats() const
{
    return m_current_stats;
}

void bitrate_calculator::reset()
{
    m_last_timestamp = utils::time::get_ticks();
    m_current_stats.reset();
    m_prev_stats.reset();
}


}
