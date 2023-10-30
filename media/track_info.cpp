#include "track_info.h"

namespace mpl::media
{

track_info_t::track_info_t(stream_id_t stream_id
                           , track_id_t track_id
                           , layer_id_t layer_id)
    : stream_id(stream_id)
    , track_id(track_id)
    , layer_id(layer_id)
{

}

bool track_info_t::operator ==(const track_info_t &other) const
{
    return stream_id == other.stream_id
            && track_id == other.track_id
            && layer_id == other.layer_id;
}

bool track_info_t::operator !=(const track_info_t &other) const
{
    return ! operator == (other);
}

bool track_info_t::is_define() const
{
    return stream_id >= stream_id_undefined;
}

}
