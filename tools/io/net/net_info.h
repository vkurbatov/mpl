#ifndef IO_NET_INFO_H
#define IO_NET_INFO_H

#include "ip_address.h"

namespace io
{

struct net_info_t
{
    std::int32_t    net_index;
    std::string     net_name;
    ip_address_t    ip_address;

    net_info_t(std::int32_t net_index = 0
            , const std::string& net_name = {}
            , const ip_address_t& ip_address = {});

    std::string to_string() const;
};
}

#endif // IO_NET_INFO_H
