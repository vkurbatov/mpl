#ifndef IO_NET_SOCKET_OPTIONS_H
#define IO_NET_SOCKET_OPTIONS_H

namespace pt::io
{

struct socket_options_t
{
    bool    reuse_address;
    bool    reuse_port;

    socket_options_t(bool reuse_address = false
                    , bool reuse_port = false);

    bool operator == (const socket_options_t& other) const;
    bool operator != (const socket_options_t& other) const;
};

}

#endif // IO_NET_SOCKET_OPTIONS_H
