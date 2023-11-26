#ifndef MPL_NET_ICE_SERVER_PARAMS_H
#define MPL_NET_ICE_SERVER_PARAMS_H

#include <string>
#include <vector>

namespace mpl::net
{

struct ice_server_params_t
{
    using array_t = std::vector<ice_server_params_t>;
    using dns_names_t = std::vector<std::string>;

    std::string     url;
    std::string     user;
    std::string     password;

    static dns_names_t get_dns_names(const ice_server_params_t::array_t& ice_servers);

    ice_server_params_t(const std::string& url = {}
                        , const std::string& user = {}
                        , const std::string& password = {});

    bool operator == (const ice_server_params_t& other) const;
    bool operator != (const ice_server_params_t& other) const;

    bool is_valid() const;

    bool parse_dns_names(std::pair<std::string, std::string>& dns_info) const;
    std::string get_dns_name() const;
};

}

#endif // MPL_NET_ICE_SERVER_PARAMS_H
