#include "event.h"

#include <vector>
#include "tools/base/sync_base.h"
#include <mutex>


namespace mpl
{

namespace detail
{
    static portable::spin_lock safe_mutex;
    static std::vector<std::string> events;

    inline event_t::event_id_t register_event(const std::string_view &event_name)
    {
        std::lock_guard lock(safe_mutex);

        event_t::event_id_t event_id = events.size();
        events.emplace_back(event_name);

        return event_id;
    }
}

event_t::event_id_t event_t::register_event(const std::string_view &event_name)
{
    return detail::register_event(event_name);
}

event_t::event_t(const event_id_t event_id
                 , const std::string_view &name)
    : event_id(event_id)
    , name(name)
{

}

event_t &event_t::operator=(const event_t &other)
{
    return *this;
}


}
