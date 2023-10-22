#include "event.h"

#include <vector>
#include "tools/utils/sync_base.h"
#include <mutex>


namespace mpl
{

bool event_t::operator ==(const event_t &other) const
{
    return event_id == other.event_id
            && name == other.name;
}

bool event_t::operator !=(const event_t &other) const
{
    return ! operator == (other);
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
