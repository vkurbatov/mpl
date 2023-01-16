#ifndef MPL_I_MESSAGE_CHANNEL_H
#define MPL_I_MESSAGE_CHANNEL_H

#include "i_channel.h"

namespace mpl
{

class i_message_sink;
class i_message_source;

class i_message_channel : public i_channel
{
public:
    using u_ptr_t = std::unique_ptr<i_message_channel>;
    using s_ptr_t = std::shared_ptr<i_message_channel>;

    virtual i_message_sink* sink() = 0;
    virtual i_message_source* source() = 0;
};

}

#endif // MPL_I_MESSAGE_CHANNEL_H
