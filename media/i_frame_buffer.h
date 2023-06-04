#ifndef MPL_MEDIA_I_FRAME_BUFFER_H
#define MPL_MEDIA_I_FRAME_BUFFER_H

#include "i_message_frame.h"

namespace mpl::media
{

class i_frame_buffer
{
public:

    using u_ptr_t = std::unique_ptr<i_frame_buffer>;
    using s_ptr_t = std::shared_ptr<i_frame_buffer>;
    using w_ptr_t = std::weak_ptr<i_frame_buffer>;

    virtual ~i_frame_buffer() = default;
    virtual bool push_frame(const i_message_frame& message_frame) = 0;
    virtual i_message_frame::u_ptr_t pop_frame() = 0;

    virtual std::size_t capacity() const = 0;
    virtual std::size_t frame_coune() const = 0;

};

}

#endif // MPL_MEDIA_I_FRAME_BUFFER_H
