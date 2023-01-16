#ifndef MPL_STREAM_INFO_H
#define MPL_STREAM_INFO_H

#include "stream_types.h"
#include <cstring>

namespace mpl::media
{

struct stream_info_t
{
    stream_id_t stream_id;
    stream_info_t(stream_id_t stream_id = stream_id_undefined);
};

}

#endif // MPL_STREAM_INFO_H
