#include "core_event_factory.h"
#include "message_event_impl.h"
#include "core/event_channel_state.h"

namespace mpl
{

core_event_factory &core_event_factory::get_instance()
{
    static core_event_factory single_factory;
    return single_factory;
}

i_message_event::u_ptr_t core_event_factory::create_message(const event_t &event)
{
    switch(event.event_id)
    {
        case event_channel_state_t::id:
            return message_event_impl<event_channel_state_t,message_class_core>::create(static_cast<const event_channel_state_t&>(event));
        break;
        default:;
    }

    return nullptr;
}

}
