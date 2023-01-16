#ifndef MPL_I_MEDIA_FRAME_H
#define MPL_I_MEDIA_FRAME_H

#include "core/time_types.h"
#include "media_types.h"
#include "core/i_buffer.h"

namespace mpl
{

class i_buffer_collection;

namespace media
{

class i_media_frame
{
public:
    using u_ptr_t = std::unique_ptr<i_media_frame>;
    using s_ptr_t = std::shared_ptr<i_media_frame>;
    virtual ~i_media_frame() = default;
    virtual media_type_t media_type() const = 0;
    virtual frame_id_t frame_id() const = 0;
    virtual timestamp_t timestamp() const = 0;
    virtual const i_buffer_collection& buffers() const = 0;
    virtual u_ptr_t clone() const = 0;
};

}

}

#endif // MPL_I_MEDIA_FRAME_H
