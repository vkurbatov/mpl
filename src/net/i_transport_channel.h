#ifndef MPL_NET_I_TRANSPORT_CHANNEL_H
#define MPL_NET_I_TRANSPORT_CHANNEL_H

#include "net_types.h"
#include "core/i_message_channel.h"
#include "core/i_parametrizable.h"

#include <memory>

namespace mpl::net
{

class i_transport_channel : public i_message_channel
        , public i_parametrizable
{
public:
    using u_ptr_t = std::unique_ptr<i_transport_channel>;
    using s_ptr_t = std::shared_ptr<i_transport_channel>;

    virtual transport_id_t transport_id() const = 0;
};

}

#endif // MPL_NET_I_TRANSPORT_CHANNEL_H
