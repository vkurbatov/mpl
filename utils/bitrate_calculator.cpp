#include "bitrate_calculator.h"

#include "time_utils.h"

namespace mpl
{

bitrate_calculator::flow_stats_t::flow_stats_t(std::size_t id
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

bool bitrate_calculator::flow_stats_t::operator ==(const flow_stats_t &other) const
{
    return id == other.id
            && total_frames == other.total_frames
            && total_bytes == other.total_bytes
            && framerate == other.framerate
            && bitrate == other.bitrate;
}

bool bitrate_calculator::flow_stats_t::operator !=(const flow_stats_t &other) const
{
    return ! operator == (other);
}

void bitrate_calculator::flow_stats_t::push_frame(std::size_t frame_size)
{
    total_frames++;
    total_bytes += frame_size;
}

void bitrate_calculator::flow_stats_t::reset()
{
    *this = {};

}

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

const bitrate_calculator::flow_stats_t &bitrate_calculator::stats() const
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
