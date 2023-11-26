#ifndef MPL_APP_EVENT_MESSAGE_FACTORY_H
#define MPL_APP_EVENT_MESSAGE_FACTORY_H

#include "core/i_event_factory.h"

namespace mpl::app
{

class event_message_factory_impl : public i_message_event_factory
{
public:
    event_message_factory_impl() = default;

    // i_message_event_factory interface
public:

    static event_message_factory_impl& get_instance();

    i_message_event::u_ptr_t create_message(const event_t &event) override;
};

}

#endif // MPL_APP_EVENT_MESSAGE_FACTORY_H
