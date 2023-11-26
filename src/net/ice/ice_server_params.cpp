#include "ice_server_params.h"

#include "utils/common_utils.h"

namespace mpl::net
{

ice_server_params_t::dns_names_t ice_server_params_t::get_dns_names(const array_t &ice_servers)
{
    dns_names_t dns_list;

    for (const auto& s : ice_servers)
    {
        auto dns_name = s.get_dns_name();
        if (!dns_name.empty())
        {
            dns_list.push_back(dns_name);
        }
    }

    return dns_list;
}

ice_server_params_t::ice_server_params_t(const std::string &url
                                         , const std::string &user
                                         , const std::string &password)
    : url(url)
    , user(user)
    , password(password)
{

}

bool ice_server_params_t::operator ==(const ice_server_params_t &other) const
{
    return url == other.url
            && user == other.user
            && password == other.password;
}

bool ice_server_params_t::operator !=(const ice_server_params_t &other) const
{
    return !operator == (other);
}

bool ice_server_params_t::is_valid() const
{
    return !url.empty();
}

bool ice_server_params_t::parse_dns_names(std::pair<std::string, std::string> &dns_info) const
{
    auto args = utils::split_lines(url, ":");

    if (args.size() == 2)
    {
        args.push_back("3478");
    }
    if (args.size() == 3
            && args[0] == "stun")
    {
        dns_info.first = args[1];
        dns_info.second = args[2];
        return true;
    }

    return false;
}

std::string ice_server_params_t::get_dns_name() const
{
    auto pos = url.find("stun:");
    if (pos == 0)
    {
        return url.substr(5);
    }

    return url;
}


}
