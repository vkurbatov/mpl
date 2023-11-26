#ifndef MPL_I_MESSAGE_EVENT_H
#define MPL_I_MESSAGE_EVENT_H

#include "i_message.h"

namespace mpl
{

struct event_t;

class i_message_event : public i_message
{
public:
    using u_ptr_t = std::unique_ptr<i_message_event>;
    using s_ptr_t = std::shared_ptr<i_message_event>;

    virtual const event_t& event() const = 0;
};

}

#endif // MPL_I_MESSAGE_EVENT_H
