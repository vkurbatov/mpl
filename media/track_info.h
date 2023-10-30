#ifndef MPL_MEDIA_TRACK_INFO_H
#define MPL_MEDIA_TRACK_INFO_H

#include "media_types.h"

namespace mpl::media
{

struct track_info_t
{
    stream_id_t     stream_id;
    track_id_t      track_id;
    layer_id_t      layer_id;

    track_info_t(stream_id_t stream_id = stream_id_undefined
                , track_id_t track_id = track_id_undefined
                , layer_id_t layer_id = layer_id_undefined);

    bool operator == (const track_info_t& other) const;
    bool operator != (const track_info_t& other) const;

    bool is_define() const;
};

}

#endif // MPL_MEDIA_TRACK_INFO_H
