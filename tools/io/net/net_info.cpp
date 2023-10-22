#include "net_info.h"

namespace pt::io
{

net_info_t::net_info_t(int32_t net_index
                       , const std::string &net_name
                       , const ip_address_t &ip_address)
    : net_index(net_index)
    , net_name(net_name)
    , ip_address(ip_address)
{

}

std::string net_info_t::to_string() const
{
    return std::string("idx: ").append(std::to_string(net_index))
            .append(", name: ").append(net_name)
            .append(", address: ").append(ip_address.to_string());
}

}
