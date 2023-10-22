#include "resolver.h"

#include "net_utils.h"
#include "tools/io/io_core.h"

#include "tools/utils/string_base.h"

#include <boost/asio/ip/udp.hpp>
#include <boost/system/error_code.hpp>

#include <queue>


namespace pt::io
{

using io_context_t = boost::asio::io_context;
using resolver_t = boost::asio::ip::udp::resolver;
using error_code_t = boost::system::error_code;
using udp_endpoint_t = boost::asio::ip::udp::endpoint;

namespace detail
{

std::pair<std::string, std::string> split_dns_names(const std::string& dns_name)
{
    auto args = pt::utils::split_lines(dns_name
                                  , ":"
                                  , 2);
    if (args.size() == 1)
    {
        return { args[0], {} };
    }
    else if (args.size() == 2)
    {
        return { args[0], args[1] };
    }

    return {};
}


}

struct resolver::pimpl_t
{
    using u_ptr_t = std::unique_ptr<pimpl_t>;
    using resolve_handler_t = resolver::resolve_handler_t;
    using resolve_info_t = resolver::resolve_info_t;

    struct async_context_t
    {
        using dns_queue_t = std::queue<std::string>;
        using u_ptr_t = std::unique_ptr<async_context_t>;
        using s_ptr_t = std::shared_ptr<async_context_t>;

        dns_queue_t                             dns_queue;
        resolve_info_t::array_t                 result_list;
        resolve_handler_t                       resolve_handler;

        static dns_queue_t create_dns_queue(const std::vector<std::string>& dns_names)
        {
            dns_queue_t dns_queue;
            for (const auto& n : dns_names)
            {
                dns_queue.push(n);
            }

            return dns_queue;
        }

        static u_ptr_t create(const std::vector<std::string>& dns_names
                              , const resolve_handler_t& resolve_handler)
        {
            if (!dns_names.empty()
                    && resolve_handler != nullptr)
            {
                return std::make_unique<async_context_t>(create_dns_queue(dns_names)
                                                         , resolve_handler);
            }

            return nullptr;
        }

        async_context_t(dns_queue_t&& dns_queue
                        , const resolve_handler_t& resolve_handler)
            : dns_queue(std::move(dns_queue))
            , resolve_handler(resolve_handler)
        {

        }
    };

    resolver_t          m_resolver;

    static u_ptr_t create(io_core &core)
    {
        return std::make_unique<pimpl_t>(core);
    }

    pimpl_t(io_core &core)
        : m_resolver(core.get<io_context_t>())
    {

    }

    ~pimpl_t()
    {
        reset();
    }

    void internal_async_resolve(async_context_t::s_ptr_t async_context)
    {
        if (!async_context->dns_queue.empty())
        {
            auto names = detail::split_dns_names(async_context->dns_queue.front());
            resolver_t::query query(names.first, names.second);

            auto native_resolver = [async_context
                                    , this](const error_code_t& error_code
                                            , resolver_t::iterator endpoint_iterator)
            {
                resolve_info_t resolve_info;

                resolve_info.error_code = error_code;
                resolve_info.dns_name = async_context->dns_queue.front();

                if (!error_code.failed())
                {
                    resolver_t::iterator end;

                    while (endpoint_iterator != end)
                    {
                        resolve_info.endpoints.emplace_back(pt::io::utils::convert<ip_endpoint_t, udp_endpoint_t>(*endpoint_iterator));
                        endpoint_iterator++;
                    }
                }

                async_context->result_list.emplace_back(std::move(resolve_info));
                async_context->dns_queue.pop();

                internal_async_resolve(std::move(async_context));
            };

            m_resolver.async_resolve(query
                                     , std::move(native_resolver));

        }
        else
        {
            async_context->resolve_handler(std::move(async_context->result_list));
        }
    }


    resolve_info_t resolve(const std::string &dns_name)
    {
        resolve_info_t result;

        auto names = detail::split_dns_names(dns_name);
        resolver_t::query query(names.first, names.second);

        error_code_t error_code;
        auto it = m_resolver.resolve(query, error_code);

        result.error_code = error_code;
        result.dns_name = dns_name;


        if (!error_code.failed())
        {
            resolver_t::iterator end;

            while (it != end)
            {
                result.endpoints.emplace_back(pt::io::utils::convert<ip_endpoint_t, udp_endpoint_t>(*it));
                it++;
            }

        }

        return result;
    }


    resolve_info_t::array_t resolve(const std::vector<std::string>& dns_names)
    {
        resolve_info_t::array_t result;

        for (const auto& n : dns_names)
        {
            result.push_back(resolve(n));
        }

        return result;
    }

    void resolve_async(const std::vector<std::string>& dns_names
                      , const resolve_handler_t& resolve_handler)
    {
        if (auto async_context = async_context_t::create(dns_names
                                                         , resolve_handler))
        {
            internal_async_resolve(std::move(async_context));
        }
    }

    void reset()
    {
        m_resolver.cancel();
    }
};

resolver::u_ptr_t resolver::create(io_core &core)
{
    return std::make_unique<resolver>(core);
}

resolver::resolver(io_core &core)
    : m_pimpl(pimpl_t::create(core))
{

}

resolver::~resolver()
{

}

resolver::resolve_info_t::array_t resolver::resolve(const std::vector<std::string> &dns_names)
{
    return m_pimpl->resolve(dns_names);
}

void resolver::resolve_async(const std::vector<std::string> &dns_names
                             , const resolve_handler_t& resolve_handler)
{
    m_pimpl->resolve_async(dns_names
                           , resolve_handler);
}

void resolver::reset()
{
    m_pimpl->reset();
}

}
