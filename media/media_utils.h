#ifndef MPL_MEDIA_UTILS_H
#define MPL_MEDIA_UTILS_H

#include "core/time_types.h"

namespace mpl
{

class i_option;
class i_property;

namespace media::utils
{

timestamp_t get_video_frame_time(double frame_rate);

bool convert_format_options(const i_option& options, i_property& property);
bool convert_format_options(const i_property& property, i_option& options);

}

}


#endif // MPL_MEDIA_UTILS_H
