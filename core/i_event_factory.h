#ifndef MPL_I_EVENT_FACTORY_H
#define MPL_I_EVENT_FACTORY_H

#include "i_message_event.h"

namespace mpl
{

class i_event_factory
{
public:
    using u_ptr_t = std::unique_ptr<i_event_factory>;
    using s_ptr_t = std::shared_ptr<i_event_factory>;

    virtual ~i_event_factory() = default;
    virtual i_message_event::u_ptr_t create_message(const event_t& event) = 0;
};

}

#endif // MPL_I_EVENT_FACTORY_H
