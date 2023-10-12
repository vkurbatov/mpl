#include "url_base.h"

#include "string_base.h"

namespace portable
{

url_info_t::url_info_t(const std::string_view &protocol
                              , const std::string_view &host
                              , const std::string_view port
                              , const std::string_view &username
                              , const std::string_view &password
                              , const std::string_view &params)
    : protocol(protocol)
    , host(host)
    , port(port)
    , username(username)
    , password(password)
    , params(params)
{

}

std::string url_info_t::to_url() const
{
    std::string url_address;
    if (is_valid())
    {
        url_address = protocol;
        url_address.append("://");

        if (!username.empty())
        {
            url_address.append(username);

            if (!password.empty())
            {
                url_address.append(":").append(password);
            }

            url_address.append("@");
        }

        url_address.append(host);

        if (!port.empty())
        {
            url_address.append(":").append(port);
        }

        if (!params.empty())
        {
            url_address.append("?").append(params);
        }
    }

    return url_address;
}

bool url_info_t::parse_url(const std::string &url_address)
{
    auto args = split_lines(url_address, "://", 1);
    if (args.size() == 2)
    {
        url_info_t tmp_info;
        tmp_info.protocol = args[0];
        args = split_lines(args[1], "@", 1);

        if (!args.empty())
        {
            if (args.size() == 2)
            {
                auto login_args = split_lines(args[0], ":", 1);

                switch(login_args.size())
                {
                    case 1u:
                        tmp_info.username = login_args[0];
                    break;
                    case 2u:
                        tmp_info.username = login_args[0];
                        tmp_info.password = login_args[1];
                    break;
                    default:;
                }
            }

            args = split_lines(args.back(), "?", 1);

            if (!args.empty())
            {
                auto host_args = split_lines(args.front(), ":", 1);
                if (!host_args.empty())
                {
                    tmp_info.host = host_args[0];

                    if (host_args.size() == 2)
                    {
                        tmp_info.port = host_args[1];
                    }

                    if (args.size() == 2)
                    {
                        tmp_info.params = args[1];
                    }

                    *this = tmp_info;
                    return true;
                }

            }
        }
    }

    return false;
}

bool url_info_t::is_valid() const
{
    return !protocol.empty()
            && !host.empty();
}

}
