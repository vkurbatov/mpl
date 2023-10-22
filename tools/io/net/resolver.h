#ifndef IO_NET_RESOLVER_H
#define IO_NET_RESOLVER_H

#include "ip_endpoint.h"

#include <memory>
#include <functional>
#include <system_error>

namespace pt::io
{

class io_core;

class resolver
{
    struct pimpl_t;
    using pimpl_ptr_t = std::unique_ptr<pimpl_t>;

    pimpl_ptr_t                 m_pimpl;

public:
    struct resolve_info_t
    {
        using array_t = std::vector<resolve_info_t>;
        std::error_code         error_code;
        std::string             dns_name;
        ip_endpoint_t::array_t  endpoints;
    };

    using resolve_handler_t = std::function<void(resolve_info_t::array_t&&)>;

    using u_ptr_t = std::unique_ptr<resolver>;

    static u_ptr_t create(io_core& core);

    resolver(io_core& core);
    ~resolver();

    resolve_info_t::array_t resolve(const std::vector<std::string>& dns_names);
    void resolve_async(const std::vector<std::string>& dns_names
                      , const resolve_handler_t& resolve_handler);

    void reset();


};

}

#endif // IO_NET_RESOLVER_H
