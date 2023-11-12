#ifndef MPL_I_MESSAGE_PACKET_H
#define MPL_I_MESSAGE_PACKET_H

#include "i_message.h"
#include "i_data_object.h"

namespace mpl
{

class i_option;

class i_message_packet : public i_message
        , public i_data_object
{
public:
    using u_ptr_t = std::unique_ptr<i_message_packet>;
    using s_ptr_t = std::shared_ptr<i_message_packet>;
    using w_ptr_t = std::weak_ptr<i_message_packet>;
    using r_ptr_t = i_message_packet*;

    virtual const i_option* options() const = 0;
};

}

#endif // MPL_I_MESSAGE_PACKET_H
