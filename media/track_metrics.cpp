#include "track_metrics.h"

namespace mpl::media
{

track_metrics_t::track_metrics_t(const media_info_t &format
                                 , const track_info_t &track_info
                                 , const flow_stats_t &flow_stats
                                 , timestamp_t timestamp)
    : format(format)
    , track_info(track_info)
    , flow_stats(flow_stats)
    , timestamp(timestamp)
{

}

bool track_metrics_t::operator ==(const track_metrics_t &other) const
{
    return format == other.format
            && track_info == other.track_info
            && flow_stats == other.flow_stats
            && timestamp == other.timestamp;
}

bool track_metrics_t::operator !=(const track_metrics_t &other) const
{
    return ! operator == (other);
}

void track_metrics_t::reset()
{
    *this = {};
}


}
