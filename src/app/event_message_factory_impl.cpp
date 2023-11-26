#include "event_message_factory_impl.h"
#include "utils/message_event_impl.h"
#include "core/event_channel_state.h"
#include "net/ice/ice_gathering_state_event.h"
#include "net/net_module_types.h"

namespace mpl::app
{

namespace detail
{

template<module_id_t ModuleId, typename Event>
i_message_event::u_ptr_t create_event_message(const Event& event)
{
    return message_event_impl<Event, ModuleId>::create(event);
}

}

event_message_factory_impl &event_message_factory_impl::get_instance()
{
    static event_message_factory_impl single_factory;
    return single_factory;
}

i_message_event::u_ptr_t event_message_factory_impl::create_message(const event_t &event)
{
    switch(event.event_id)
    {
        case event_channel_state_t::id:
            return detail::create_event_message<core_module_id>(static_cast<const event_channel_state_t&>(event));
        break;
        case net::ice_gathering_state_event_t::id:
            return detail::create_event_message<net::net_module_id>(static_cast<const net::ice_gathering_state_event_t&>(event));
        break;
        default:;
    }

    return nullptr;
}

}
