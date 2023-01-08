#include "media_utils.h"
#include "media_types.h"

namespace mpl::utils
{

timestamp_t get_video_frame_time(double frame_rate)
{
    if (frame_rate != 0.0)
    {
        return video_sample_rate / frame_rate;
    }

    return timestamp_null;
}

}
