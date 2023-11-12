#ifndef MPL_I_MEDIA_FRAME_H
#define MPL_I_MEDIA_FRAME_H

#include "core/time_types.h"
#include "i_message_media_data.h"

namespace mpl
{

namespace media
{

class i_media_frame : public i_message_media_data
{
public:
    using u_ptr_t = std::unique_ptr<i_media_frame>;
    using s_ptr_t = std::shared_ptr<i_media_frame>;
    virtual ~i_media_frame() = default;
    virtual media_type_t media_type() const = 0;
    virtual frame_id_t frame_id() const = 0;
    virtual timestamp_t timestamp() const = 0;
    virtual timestamp_t ntp_timestamp() const = 0;
};

}

}

#endif // MPL_I_MEDIA_FRAME_H
