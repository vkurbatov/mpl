#include "udp_link_config.h"

namespace io
{


udp_link_config_t::udp_link_config_t(const socket_options_t& socket_options)
    : link_config_t(link_type_t::udp)
    , socket_options(socket_options)
{

}

bool udp_link_config_t::is_valid() const
{
    return type == link_type_t::udp;
}

}
