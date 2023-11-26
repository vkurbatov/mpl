#ifndef MPL_MEDIA_FRAME_OPTION_CONTAINER_H
#define MPL_MEDIA_FRAME_OPTION_CONTAINER_H

#include "utils/option_container.h"
#include "media_types.h"

namespace mpl::media
{

class frame_option_container : public utils::option_container
{
public:
    frame_option_container(const i_option& options);
    frame_option_container(option_impl&& options = {});

    stream_id_t stream_id() const;
    track_id_t track_id() const;
    layer_id_t layer_id() const;

    bool set_stream_id(stream_id_t stream_id);
    bool set_track_id(stream_id_t track_id);
    bool set_layer_id(stream_id_t layer_id);
};

}

#endif // MPL_MEDIA_FRAME_OPTION_CONTAINER_H
