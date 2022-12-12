#include "vnc_base.h"

#include "tools/base/string_base.h"

namespace vnc
{

vnc_server_config_t vnc_server_config_t::from_uri(const std::string &uri)
{
    vnc_server_config_t vnc_server_config;

    if (uri.find("vnc://") == 0)
    {
        auto begin = 6;
        auto end = uri.find('@', begin);

        if (end != std::string::npos)
        {
            vnc_server_config.password = uri.substr(begin
                                                    , end - begin);
            begin = end + 1;
        }

        end = uri.find(':', begin);

        if (end != std::string::npos)
        {
            vnc_server_config.host = uri.substr(begin
                                                , end - begin);
            vnc_server_config.port = atoi(uri.substr(end + 1).c_str());
        }
        else
        {
            vnc_server_config.host = uri.substr(begin);
        }

        if (vnc_server_config.port == 0)
        {
            vnc_server_config.port = default_port;
        }
    }

    return vnc_server_config;
}

vnc_server_config_t::vnc_server_config_t(const std::string &host
                                         , const std::string &password
                                         , uint32_t port)
    : host(host)
    , password(password)
    , port(port)
{

}

std::string vnc_server_config_t::uri() const
{
    std::string vnc_uri;

    vnc_uri.reserve(password.size() + host.size() + 32);

    vnc_uri.append("vnc://");

    if (!password.empty())
    {
        vnc_uri.append(password);
        vnc_uri.append("@");
    }

    vnc_uri.append(host);
    vnc_uri.append(":");
    vnc_uri.append(std::to_string(port));

    return vnc_uri;
}

frame_t::frame_t(const frame_size_t frame_size
                 , std::uint32_t fps
                 , uint32_t bpp)
    : frame_size(frame_size)
    , fps(fps)
    , bpp(bpp)
{
    realloc();
}

std::size_t frame_t::realloc()
{
    frame_data.resize((frame_size.size() * bpp) / 8);
    return frame_data.size();
}

vnc_config_t::vnc_config_t(uint32_t fps)
    : fps(fps)
{

}


}
