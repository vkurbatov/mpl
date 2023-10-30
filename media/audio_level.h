#ifndef MPL_AUDIO_LEVEL_H
#define MPL_AUDIO_LEVEL_H

#include "core/time_types.h"

namespace mpl::media
{

struct audio_info_t;

class audio_level
{
public:
    static constexpr timestamp_t default_update_frequency = 10;
    struct config_t
    {
        timestamp_t update_frequency;
        config_t(timestamp_t update_frequency = default_update_frequency);
    };

private:

    config_t        m_config;

    double          m_abs_max;
    std::size_t     m_count;
    double          m_level;

    double          m_total_energy;
    timestamp_t     m_total_duration;

public:
    audio_level(const config_t& config = {});

    bool push_frame(const audio_info_t& sample_info
                    , const void* data
                    , std::size_t samples);
    void reset(bool only_level = false);

    double max_level() const;
    double level() const;
    double total_energy() const;
    timestamp_t total_duration() const;
};

}

#endif // MPL_AUDIO_LEVEL_H
