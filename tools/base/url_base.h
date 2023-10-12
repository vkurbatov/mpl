#ifndef PORTABLE_URL_H
#define PORTABLE_URL_H

#include <string>

namespace portable
{

struct url_info_t
{
    std::string protocol;
    std::string host;
    std::string port;
    std::string username;
    std::string password;
    std::string params;

    url_info_t(const std::string_view& protocol = {}
               , const std::string_view& host = {}
               , const std::string_view port = {}
               , const std::string_view& username = {}
               , const std::string_view& password = {}
               , const std::string_view& params = {});

    std::string to_url() const;
    bool parse_url(const std::string& url_address);

    bool is_valid() const;
};


}

#endif // PORTABLE_URL_H
