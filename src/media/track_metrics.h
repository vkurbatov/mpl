#ifndef MPL_MEDIA_TRACK_METRICS_H
#define MPL_MEDIA_TRACK_METRICS_H

#include "track_info.h"
#include "media_info.h"
#include "core/flow_stats.h"

namespace mpl::media
{

struct track_metrics_t
{
    using array_t = std::vector<track_metrics_t>;

    media_info_t    format;
    track_info_t    track_info;
    flow_stats_t    flow_stats;
    timestamp_t     timestamp;

    track_metrics_t(const media_info_t& format = {}
                    , const track_info_t& track_info = {}
                    , const flow_stats_t& flow_stats = {}
                    , timestamp_t timestamp = timestamp_null);

    bool operator == (const track_metrics_t& other) const;
    bool operator != (const track_metrics_t& other) const;

    void reset();


};

}

#endif // MPL_MEDIA_TRACK_METRICS_H
