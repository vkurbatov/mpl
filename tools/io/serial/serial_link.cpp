#include "serial_link.h"
#include "serial_link_config.h"
#include "serial_endpoint.h"
#include "tools/io/io_core.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

#include <atomic>
#include <vector>
#include <array>

#include <thread>

namespace pt::io
{

constexpr static std::size_t recv_buffer_size = 1000;
using serial_t = boost::asio::serial_port;
using io_contex_t = boost::asio::io_context;
using array_t = std::array<std::uint8_t, recv_buffer_size>;

namespace detail
{

bool set_link_params(serial_t& serial
                      , const serial_link_config_t& config
                      , boost::system::error_code& ec)
{
    if (config.is_valid())
    {
        serial.set_option(boost::asio::serial_port_base::baud_rate(config.baud_rate), ec);

        if (!ec.failed())
        {
            serial.set_option(boost::asio::serial_port_base::character_size(config.char_size), ec);
        }

        if (!ec.failed())
        {
            serial.set_option(boost::asio::serial_port_base::stop_bits(static_cast<boost::asio::serial_port_base::stop_bits::type>(config.stop_bits)), ec);
        }

        if (!ec.failed())
        {
            serial.set_option(boost::asio::serial_port_base::parity(static_cast<boost::asio::serial_port_base::parity::type>(config.parity)), ec);
        }

        if (!ec.failed())
        {
            serial.set_option(boost::asio::serial_port_base::flow_control(static_cast<boost::asio::serial_port_base::flow_control::type>(config.flow_control)), ec);
        }

        return !ec.failed();
    }

    return false;
}

}

struct serial_link::pimpl_t
{
    using u_ptr_t = serial_link::pimpl_ptr_t;

    serial_link&            m_link;
    serial_link_config_t    m_config;
    serial_endpoint_t       m_endpoint;
    serial_t                m_serial;
    array_t                 m_recv_buffer;

    bool                    m_started;
    std::atomic_bool        m_io_processed;

    static u_ptr_t create(serial_link& link
                          , const serial_link_config_t& config)
    {
        return std::make_unique<pimpl_t>(link
                                         , config);
    }

    pimpl_t(serial_link& link
            , const serial_link_config_t& config)
        : m_link(link)
        , m_config(config)
        , m_serial(m_link.m_core.get<io_contex_t>())
        , m_started(false)
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

    bool open()
    {
        if (!m_serial.is_open())
        {
            boost::system::error_code ec;
            change_state(link_state_t::opening);

            if (m_endpoint.is_valid())
            {
                m_serial.open(m_endpoint.port_name, ec);

                if (!ec.failed())
                {
                    if (detail::set_link_params(m_serial
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
        if (m_serial.is_open())
        {
            change_state(link_state_t::closing);
            stop();
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
            m_serial.cancel();
            while(m_io_processed.load(std::memory_order_relaxed))
            {
                std::this_thread::yield();
            }
            change_state(link_state_t::disconnected);
            return true;
        }

        return false;
    }

    inline bool set_config(const serial_link_config_t &config)
    {
        if (!is_open())
        {
            m_config = config;
            return true;
        }

        return false;
    }

    inline bool set_endpoint(const endpoint_t &endpoint)
    {
        if (!is_open())
        {
            if (endpoint.type == endpoint_t::type_t::serial)
            {
                m_endpoint = static_cast<const serial_endpoint_t&>(endpoint);
                return true;
            }
        }

        return false;
    }

    inline bool is_open()
    {
        return m_serial.is_open();
    }


    inline bool send_to(const message_t &message
                        , const endpoint_t& /*endpoint*/)
    {
        if (is_open()
                && m_started)
        {
            auto buffer = boost::asio::buffer(message.data()
                                               , message.size());
            m_serial.async_write_some(buffer
                                      , [&](auto&&... args) { on_send(args...); });
            return true;
        }

        return false;
    }

    inline void on_send(boost::system::error_code error_code
                        , std::size_t bytes_transfered)
    {
        if (error_code.failed())
        {
            change_state(link_state_t::failed
                         , error_code.message());
            m_started = false;
        }
        // nothing
    }

    inline void do_receive()
    {
        if (m_started)
        {
            m_io_processed.store(true);
            auto buffer = boost::asio::buffer(m_recv_buffer);
            m_serial.async_read_some(buffer
                                     , [&](auto&&... args) { on_receive(args...); });
            return;

        }
        m_io_processed.store(false
                             , std::memory_order_release);
    }

    inline void on_receive(boost::system::error_code error_code
                           , std::size_t bytes_transfered)
    {
        if (error_code.failed())
        {
            change_state(link_state_t::failed
                         , error_code.message());
        }
        else
        {
            message_t message(m_recv_buffer.data()
                              , bytes_transfered);

            m_link.on_recv_message(message
                                   , m_endpoint);

            do_receive();
            return;
        }
        m_io_processed.store(false
                             , std::memory_order_release);
    }

};

serial_link::u_ptr_t serial_link::create(io_core &core
                                         , const serial_link_config_t &config)
{
    return std::make_unique<serial_link>(core
                                         , config);
}

serial_link::serial_link(io_core &core
                         , const serial_link_config_t &config)
    : io_link(core)
    , m_pimpl(pimpl_t::create(*this
                              , config))
{

}

serial_link::~serial_link()
{

}

bool serial_link::set_config(const serial_link_config_t &config)
{
    return m_pimpl->set_config(config);
}

const serial_link_config_t &serial_link::config()
{
    return m_pimpl->m_config;
}

const serial_endpoint_t &serial_link::endpoint() const
{
    return m_pimpl->m_endpoint;
}

link_type_t serial_link::type() const
{
    return link_type_t::serial;
}

bool serial_link::control(link_control_id_t control_id)
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

bool serial_link::send_to(const message_t &message
                          , const endpoint_t &endpoint)
{
    return m_pimpl->send_to(message
                            , endpoint);
}

bool serial_link::get_endpoint(endpoint_t &endpoint) const
{
    if (endpoint.type == endpoint_t::type_t::serial)
    {
        static_cast<serial_endpoint_t&>(endpoint) = m_pimpl->m_endpoint;
        return true;
    }

    return false;
}

bool serial_link::set_endpoint(const endpoint_t &endpoint)
{
    return m_pimpl->set_endpoint(endpoint);
}

bool serial_link::is_open() const
{
    return m_pimpl->is_open();
}

}
