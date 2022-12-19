#ifndef MPL_STREAM_INFO_H
#define MPL_STREAM_INFO_H

#include "media_types.h"
#include <cstring>

namespace mpl
{

class stream_info_t
{
    stream_id_t stream_id;
    stream_info_t(stream_id_t stream_id = stream_id_undefined);
};

}

#endif // MPL_STREAM_INFO_H