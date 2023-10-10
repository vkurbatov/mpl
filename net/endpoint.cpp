#include "endpoint.h"

namespace mpl::net
{

endpoint_t::endpoint_t()
    : endpoint_t(transport_id_t::undefined)
{

}

endpoint_t &endpoint_t::operator=(const endpoint_t &other)
{
    return *this;
}

bool endpoint_t::operator ==(const endpoint_t &other) const
{
    return other == other.transport_id;
}

bool endpoint_t::operator !=(const endpoint_t &other) const
{
    return ! operator == (other);
}

endpoint_t::endpoint_t(transport_id_t transport_id)
    : transport_id(transport_id)
{

}

}
