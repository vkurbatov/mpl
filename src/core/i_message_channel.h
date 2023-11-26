#ifndef MPL_I_MESSAGE_CHANNEL_H
#define MPL_I_MESSAGE_CHANNEL_H

#include "i_message_transceiver.h"
#include "i_channel.h"

namespace mpl
{

class i_message_channel : public i_channel
        , public i_message_transceiver
{
public:
    using u_ptr_t = std::unique_ptr<i_message_channel>;
    using s_ptr_t = std::shared_ptr<i_message_channel>;
};

}

#endif // MPL_I_MESSAGE_CHANNEL_H
