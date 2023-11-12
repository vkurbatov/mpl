#include "endpoint.h"

namespace pt::io
{

const endpoint_t &endpoint_t::undefined()
{
    static const endpoint_t single_undefined_endpoint;
    return single_undefined_endpoint;
}

endpoint_t::endpoint_t()
    : endpoint_t(type_t::undefined)
{

}


endpoint_t::endpoint_t(type_t type)
    : type(type)
{

}

endpoint_t &endpoint_t::operator =(const endpoint_t &other)
{
    return *this;
}

bool endpoint_t::operator ==(const endpoint_t &other) const
{
    return type == other.type;
}

bool endpoint_t::operator !=(const endpoint_t &other) const
{
    return ! operator == (other);
}

bool endpoint_t::is_valid() const
{
    return true;
}

std::string endpoint_t::to_string() const
{
    return std::string("undefined");
}

}
