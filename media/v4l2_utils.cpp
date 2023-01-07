#include "v4l2_utils.h"
#include <unordered_map>

#include <linux/videodev2.h>

namespace mpl::utils
{

namespace detail
{

using format_map_t = std::unordered_map<v4l2::pixel_format_t, video_format_id_t>;


}
