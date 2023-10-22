#ifndef IO_NET_UDP_LINK_CONFIG_H
#define IO_NET_UDP_LINK_CONFIG_H

#include "tools/io/io_base.h"
#include "socket_options.h"

namespace pt::io
{

struct udp_link_config_t : public link_config_t
{
    socket_options_t    socket_options;

    udp_link_config_t(const socket_options_t& socket_options = {});

    bool is_valid() const override;
};

}

#endif // IO_NET_UDP_LINK_CONFIG_H
