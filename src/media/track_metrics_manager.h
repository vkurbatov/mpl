#ifndef MPL_MEDIA_TRACK_METRICS_MANAGER_H
#define MPL_MEDIA_TRACK_METRICS_MANAGER_H

#include "core/i_message.h"
#include "track_metrics.h"
#include "utils/bitrate_calculator.h"
#include <unordered_map>
#include "utils/watch_timer.h"

namespace mpl::media
{

class i_media_frame;

class track_metrics_manager
{
public:
    struct config_t
    {
        timestamp_t     check_interval = durations::seconds(1);
    };
private:
    struct track_t
    {
        using map_t = std::unordered_map<track_info_t, track_t>;

        utils::bitrate_calculator   bitrate_calculator;
        track_metrics_t             metrics;

        track_t(const track_info_t& track_info);

        void push_frame(const i_media_frame& media_frame);
        void reset();
    };

    config_t                m_config;
    track_t::map_t          m_tracks;
    watch_timer             m_check_timer;

public:
    track_metrics_manager(const config_t& config);

    track_metrics_t::array_t get_metrics() const;
    void reset();

    void push_message(const i_message& message);

private:
    std::size_t remove_garbage();
    track_t* query_track(const i_media_frame& media_frame);
};

}

#endif // MPL_MEDIA_TRACK_METRICS_MANAGER_H
