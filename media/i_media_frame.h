#ifndef MPL_I_MEDIA_FRAME_H
#define MPL_I_MEDIA_FRAME_H

#include "i_buffer.h"

namespace mpl
{

class i_media_format;
class stream_info_t;

class i_media_frame
{
public:
    using u_ptr_t = std::unique_ptr<i_media_frame>;
    using s_ptr_t = std::shared_ptr<i_media_frame>;
    virtual ~i_media_frame() = default;
    virtual const i_media_format& format() const = 0;
    virtual const i_buffer* get_buffer(std::int32_t buffer_index = 0) const = 0;
    virtual std::size_t buffers() const = 0;
    virtual const stream_info_t& stream_info() const = 0;
    virtual u_ptr_t clone() const = 0;

};

}

#endif // MPL_I_MEDIA_FRAME_H
