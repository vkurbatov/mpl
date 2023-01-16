#include "event.h"

namespace mpl
{

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

event_t &event_t::operator=(event_t &&other)
{
    return *this;
}

}
