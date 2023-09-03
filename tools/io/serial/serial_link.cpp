#include "serial_link.h"
#include "serial_link_config.h"
#include "serial_endpoint.h"
#include "tools/io/io_core.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

namespace io
{

using serial_t = boost::asio::serial_port;
using io_contex_t = boost::asio::io_context;

namespace detail
{

bool set_seral_params(serial_t& serial
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

    bool                    m_started;

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
                    if (detail::set_seral_params(m_serial
                                                 , m_config
                                                 , ec))
                    {
                        change_state(link_state_t::open);
                        return true;
                    }
                }
            }

            close();

            if (!ec.failed())
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
            stop();
            return true;
        }

        return false;
    }

    bool start()
    {
        if (is_open()
                || open())
        {
            return true;
        }

        return false;
    }

    bool stop()
    {
        if (is_open())
        {

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
            if (endpoint.type == endpoint_type_t::serial)
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
    return false;
}

bool serial_link::set_endpoint(const endpoint_t &endpoint)
{
    return false;
}

bool serial_link::is_open() const
{
    return m_pimpl->is_open();
}

}
