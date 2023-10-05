#include "socket_options.h"

namespace io
{


socket_options_t::socket_options_t(bool reuse_address
                                   , bool reuse_port)
    : reuse_address(reuse_address)
    , reuse_port(reuse_port)
{

}

bool socket_options_t::operator ==(const socket_options_t &other) const
{
    return reuse_address == other.reuse_address
            && reuse_port == other.reuse_port;
}

bool socket_options_t::operator !=(const socket_options_t &other) const
{
    return ~ operator == (other);
}

}
