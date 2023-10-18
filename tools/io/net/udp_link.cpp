#include "udp_link.h"
#include "ip_endpoint.h"
#include "udp_link_config.h"
#include "tools/io/io_core.h"

#include "net_utils.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

#include <atomic>
#include <vector>
#include <array>

#include <thread>

#include <iostream>

namespace io
{

constexpr static std::size_t recv_buffer_size = 10000;
constexpr static std::size_t max_errors = 5;
using socket_t = boost::asio::ip::udp::socket;
using udp_endpoint_t = boost::asio::ip::udp::endpoint;
using io_contex_t = boost::asio::io_context;
using array_t = std::array<std::uint8_t, recv_buffer_size>;

namespace detail
{

bool set_link_params(socket_t& socket
                    , const udp_link_config_t& config
                    , boost::system::error_code& ec)
{
    if (config.is_valid())
    {
        return !ec.failed();
    }
    return false;
}

}

struct udp_link::pimpl_t
{
    using u_ptr_t = udp_link::pimpl_ptr_t;

    udp_link&               m_link;
    udp_link_config_t       m_config;
    ip_endpoint_t           m_local_endpoint;
    ip_endpoint_t           m_remote_endpoint;

    udp_endpoint_t          m_recv_endpoint;

    socket_t                m_socket;
    array_t                 m_recv_buffer;

    bool                    m_started;
    std::atomic_bool        m_io_processed;
    std::size_t             m_repeating_errors;

    static u_ptr_t create(udp_link& link
                          , const udp_link_config_t& config)
    {
        return std::make_unique<pimpl_t>(link
                                         , config);
    }

    pimpl_t(udp_link& link
            , const udp_link_config_t& config)
        : m_link(link)
        , m_config(config)
        , m_socket(m_link.m_core.get<io_contex_t>())
        , m_started(false)
        , m_io_processed(false)
        , m_repeating_errors(0)
    {

    }

    ~pimpl_t()
    {
        close();
    }


    inline void change_state(link_state_t state
                             , const std::string_view& reason = {})
    {
        m_link.change_state(state
                            , reason);
    }

    bool socket_open(boost::system::error_code& ec)
    {
        switch(m_local_endpoint.address.version)
        {
            case ip_version_t::ip4:
                m_socket.open(boost::asio::ip::udp::v4()
                              , ec);
            break;
            case ip_version_t::ip6:
                m_socket.open(boost::asio::ip::udp::v6()
                              , ec);
            break;
            default:;
        }

        if (!ec.failed()
                && m_socket.is_open())
        {
            if (m_config.socket_options.reuse_address)
            {
                m_socket.set_option(boost::asio::ip::udp::socket::reuse_address(true)
                                   , ec);
            }

            if (!ec.failed())
            {
                m_socket.bind(utils::convert<udp_endpoint_t>(m_local_endpoint)
                              , ec);

                if (!ec.failed())
                {
                    m_local_endpoint = utils::convert<ip_endpoint_t>(m_socket.local_endpoint());
                    return true;
                }
            }

            m_socket.close();
        }

        return false;
    }

    bool open()
    {
        if (!m_socket.is_open())
        {
            boost::system::error_code ec;
            change_state(link_state_t::opening);

            if (m_local_endpoint.is_valid())
            {
                if (socket_open(ec))
                {
                    if (detail::set_link_params(m_socket
                                                 , m_config
                                                 , ec))
                    {
                        change_state(link_state_t::open);
                        return true;
                    }
                }
            }

            close();

            if (ec.failed())
            {
                change_state(link_state_t::failed
                             , ec.message());
            }
        }

        return false;
    }

    bool close()
    {
        if (m_socket.is_open())
        {
            change_state(link_state_t::closing);
            stop();
            m_socket.close();
            change_state(link_state_t::closed);
            return true;
        }

        return false;
    }

    bool start()
    {
        if (!m_started
                && is_open())
        {
            m_repeating_errors = 0;
            change_state(link_state_t::connecting);
            m_started = true;
            change_state(link_state_t::connected);
            do_receive();
            return true;
        }

        return false;
    }

    bool stop()
    {
        if (m_started)
        {
            change_state(link_state_t::disconnecting);
            m_started = false;
            m_socket.cancel();
            while(m_io_processed.load(std::memory_order_relaxed))
            {
                std::this_thread::yield();
            }
            change_state(link_state_t::disconnected);
            return true;
        }

        return false;
    }

    inline bool set_config(const udp_link_config_t &config)
    {
        if (!is_open())
        {
            m_config = config;
            return true;
        }

        return false;
    }

    inline bool set_endpoint(const ip_endpoint_t &endpoint
                             , bool remote = false)
    {
        (remote ? m_remote_endpoint : m_local_endpoint) = endpoint;
        return true;
    }


    inline bool is_open()
    {
        return m_socket.is_open();
    }


    inline bool send_to(const message_t &message
                        , const ip_endpoint_t& endpoint)
    {
        if (is_open()
                && m_started)
        {
            auto buffer = boost::asio::buffer(message.data()
                                               , message.size());

            m_socket.async_send_to(buffer
                                   , utils::convert<udp_endpoint_t>(endpoint)
                                   , [&](auto&&... args) { on_send(args...); });
            return true;
        }

        return false;
    }

    inline bool send(const message_t &message)
    {
        if (m_remote_endpoint.is_valid())
        {
            return send_to(message
                           , m_remote_endpoint);
        }
        return false;
    }

    inline void on_send(boost::system::error_code error_code
                        , std::size_t bytes_transfered)
    {
        if (m_started)
        {
            if (error_code.failed())
            {
                process_error(error_code);
                // std::clog << "udp send: failed: " << error_code.message() << std::endl;
            }
            else
            {
                m_repeating_errors = 0;
            }
        }
        // nothing
    }

    inline void do_receive()
    {
        if (m_started)
        {
            m_io_processed.store(true, std::memory_order_release);
            auto buffer = boost::asio::buffer(m_recv_buffer);
            m_socket.async_receive_from(buffer
                                        , m_recv_endpoint
                                        , 0
                                        , [&](auto&&... args) { on_receive(args...); });

        }
        else
        {
            m_io_processed.store(false
                                , std::memory_order_release);
        }
    }

    inline void on_receive(boost::system::error_code error_code
                           , std::size_t bytes_transfered)
    {
        if (m_started)
        {
            if (error_code.failed())
            {
                process_error(error_code);
                if (m_started)
                {
                    do_receive();
                    return;
                }
            }
            else
            {
                m_repeating_errors = 0;
                message_t message(m_recv_buffer.data()
                                  , bytes_transfered);

                m_link.on_recv_message(message
                                       , utils::convert<ip_endpoint_t>(m_recv_endpoint));

                do_receive();
                return;
            }
        }

        m_io_processed.store(false
                             , std::memory_order_release);
    }

    inline void process_error(const boost::system::error_code& error_code)
    {
        m_repeating_errors++;
        if (m_repeating_errors >= max_errors)
        {
            change_state(link_state_t::failed
                         , error_code.message());
            m_repeating_errors = 0;
            m_started = false;
        }
    }

};

udp_link::u_ptr_t udp_link::create(io_core &core
                                         , const udp_link_config_t &config)
{
    return std::make_unique<udp_link>(core
                                      , config);
}

udp_link::udp_link(io_core &core
                   , const udp_link_config_t &config)
    : io_link(core)
    , m_pimpl(pimpl_t::create(*this
                              , config))
{

}

udp_link::~udp_link()
{

}

bool udp_link::set_config(const udp_link_config_t &config)
{
    return m_pimpl->set_config(config);
}

const udp_link_config_t &udp_link::config()
{
    return m_pimpl->m_config;
}

const ip_endpoint_t& udp_link::local_endpoint() const
{
    return m_pimpl->m_local_endpoint;
}

const ip_endpoint_t& udp_link::remote_endpoint() const
{
    return m_pimpl->m_remote_endpoint;
}

bool udp_link::set_local_endpoint(const ip_endpoint_t &endpoint)
{
    return m_pimpl->set_endpoint(endpoint, false);
}

bool udp_link::set_remote_endpoint(const ip_endpoint_t &endpoint)
{
    return m_pimpl->set_endpoint(endpoint, true);
}

link_type_t udp_link::type() const
{
    return link_type_t::serial;
}

bool udp_link::control(link_control_id_t control_id)
{
    switch(control_id)
    {
        case link_control_id_t::open:
            return m_pimpl->open();
        break;
        case link_control_id_t::close:
            return m_pimpl->close();
        break;
        case link_control_id_t::start:
            return m_pimpl->start();
        break;
        case link_control_id_t::stop:
            return m_pimpl->stop();
        break;
        default:;
    }

    return false;
}

bool udp_link::send_to(const message_t &message
                          , const endpoint_t &endpoint)
{
    return endpoint.type == endpoint_t::type_t::undefined
            ? m_pimpl->send(message)
            : m_pimpl->send_to(message
                               , static_cast<const ip_endpoint_t&>(endpoint));
}

bool udp_link::get_endpoint(endpoint_t &endpoint) const
{
    if (endpoint.type == endpoint_t::type_t::ip)
    {
        static_cast<ip_endpoint_t&>(endpoint) = m_pimpl->m_local_endpoint;
        return true;
    }

    return false;
}

bool udp_link::set_endpoint(const endpoint_t &endpoint)
{
    if (endpoint.type == endpoint_t::type_t::ip)
    {
        return m_pimpl->set_endpoint(static_cast<const ip_endpoint_t&>(endpoint));
    }

    return false;
}

bool udp_link::is_open() const
{
    return m_pimpl->is_open();
}

}
