#ifndef MPL_MEDIA_UTILS_H
#define MPL_MEDIA_UTILS_H

#include "core/time_types.h"

namespace mpl::media
{

class i_audio_format;
class i_video_format;

}

namespace mpl
{

class i_option;
class i_property;

namespace utils
{

timestamp_t get_video_frame_time(double frame_rate);


bool convert_format_options(const i_option& options, i_property& property);
bool convert_format_options(const i_property& property, i_option& options);

} // utils


} // mpl


#endif // MPL_MEDIA_UTILS_H