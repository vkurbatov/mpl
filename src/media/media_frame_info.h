#ifndef MPL_MEDIA_FRAME_INFO_H
#define MPL_MEDIA_FRAME_INFO_H

#include "core/time_types.h"
#include "media_types.h"

namespace mpl::media
{

struct media_frame_info_t
{
    frame_id_t      frame_id;
    timestamp_t     timestamp;
    timestamp_t     ntp_timestamp;

    media_frame_info_t(frame_id_t frame_id = frame_id_undefined
                       , timestamp_t timestamp = timestamp_null);

    media_frame_info_t(frame_id_t frame_id
                       , timestamp_t timestamp
                       , timestamp_t ntp_timestamp);

    bool operator == (const media_frame_info_t& other) const;
    bool operator != (const media_frame_info_t& other) const;
};

}

#endif // MPL_MEDIA_FRAME_INFO_H
