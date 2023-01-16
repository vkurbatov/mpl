#ifndef MPL_I_MESSAGE_FRAME_H
#define MPL_I_MESSAGE_FRAME_H

#include "core/i_message.h"

namespace mpl
{

class i_media_frame;
class i_option;
class stream_info_t;

class i_message_frame : public i_message
{
public:
    using u_ptr_t = std::unique_ptr<i_message_frame>;
    using s_ptr_t = std::shared_ptr<i_message_frame>;

    virtual const i_media_frame& frame() const = 0;
    virtual const i_option& options() const = 0;
};

}

#endif // MPL_I_MESSAGE_FRAME_H
