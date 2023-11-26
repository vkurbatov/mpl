#ifndef IO_NET_UTILS_H
#define IO_NET_UTILS_H

#include <string>
#include "ip_address.h"
#include "net_info.h"
#include <vector>


namespace pt::io::utils
{

template<typename Tout, typename Tin>
Tout convert(const Tin& input);

std::string get_host_name();

ip_address_t get_host_by_name(const std::string_view& name
                              , ip_version_t ip_version = ip_version_t::undefined);

std::vector<ip_address_t> get_local_address_list(ip_version_t ip_version = ip_version_t::undefined);

std::vector<net_info_t> get_net_info(ip_version_t ip_version = ip_version_t::undefined);

}

#endif // IO_NET_UTILS_H
