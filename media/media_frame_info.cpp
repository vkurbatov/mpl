#include "media_frame_info.h"
#include "utils/time_utils.h"

namespace mpl::media
{


media_frame_info_t::media_frame_info_t(frame_id_t frame_id
                                       , timestamp_t timestamp)
    : media_frame_info_t(frame_id
                         , timestamp
                         , utils::time::now())
{

}

media_frame_info_t::media_frame_info_t(frame_id_t frame_id
                                       , timestamp_t timestamp
                                       , timestamp_t ntp_timestamp)
    : frame_id(frame_id)
    , timestamp(timestamp)
    , ntp_timestamp(ntp_timestamp)
{

}

bool media_frame_info_t::operator ==(const media_frame_info_t &other) const
{
    return frame_id == other.frame_id
            && timestamp == other.timestamp
            && ntp_timestamp == other.ntp_timestamp;
}

bool media_frame_info_t::operator !=(const media_frame_info_t &other) const
{
    return ! operator == (other);
}

}
