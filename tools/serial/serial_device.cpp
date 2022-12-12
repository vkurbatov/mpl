#include "serial_device.h"
#include "serial_api.h"

namespace serial
{



struct serial_device_context_t
{
    control_config_t    m_control_config;
    handle_t            m_handle;

    serial_device_context_t(const control_config_t& control_config)
        : m_control_config(control_config)
        , m_handle(no_handle)
    {

    }

    ~serial_device_context_t()
    {
        close();
    }


    bool open(const std::string &serial_name)
    {
        serial_close(m_handle);

        m_handle = serial_open(serial_name.c_str()
                               , m_control_config);

        return is_opened();
    }

    bool close()
    {
        auto result = serial_close(m_handle);

        m_handle = no_handle;

        return result;
    }

    bool is_opened() const
    {
        return is_valid_handle(m_handle);
    }

    bool is_established() const
    {
        return is_opened();
    }

    bool set_config(const control_config_t& control_config)
    {
        if (!is_opened()
                || serial_set_config(m_handle
                                     , control_config))
        {
            m_control_config = control_config;
            return true;
        }

        return false;
    }

    std::size_t write(const void *data
                      , std::size_t size)
    {
        return serial_write(m_handle
                           , data
                           , size);
    }

    std::size_t read(void *data
                     , std::size_t size
                     , timeout_t timeout)
    {
        return serial_read(m_handle
                           , data
                           , size
                           , timeout);
    }

    std::size_t read(frame_data_t &frame_data
                     , bool is_append
                     , timeout_t timeout)
    {
        return serial_read(m_handle
                           , frame_data
                           , is_append
                           , timeout);
    }

    frame_data_t read(timeout_t timeout)
    {
        frame_data_t frame_data;

        serial_read(m_handle
                    , frame_data
                    , false
                    , timeout);

        return std::move(frame_data);
    }

    bool flush()
    {
        return serial_flush(m_handle);
    }


    std::size_t get_ready_data_size()
    {
        return serial_unread_data_size(m_handle);
    }
};

//---------------------------------------------------------------------------------------------
void serial_device_context_deleter_t::operator()(serial_device_context_t *serial_device_context_ptr)
{
    delete serial_device_context_ptr;
}
//---------------------------------------------------------------------------------------------
serial_list_t serial_device::serial_devices()
{
    return serial::serial_list();
}

serial_device::serial_device(const control_config_t& control_config)
    : m_serial_device_context(new serial_device_context_t(control_config))
{

}

bool serial_device::open(const std::string &serial)
{
    return m_serial_device_context->open(serial);
}

bool serial_device::close()
{
    return m_serial_device_context->close();
}

bool serial_device::is_opened() const
{
    return m_serial_device_context->is_opened();
}

bool serial_device::is_established() const
{
    return m_serial_device_context->is_established();
}

const control_config_t &serial_device::config() const
{
    return m_serial_device_context->m_control_config;
}

bool serial_device::set_config(const control_config_t &control_config)
{
    if (!m_serial_device_context->is_opened())
    {
        m_serial_device_context.reset(new serial_device_context_t(control_config));
        return true;
    }

    return false;
}

std::size_t serial_device::write(const void *data
                                 , std::size_t size)
{
    return m_serial_device_context->write(data
                                          , size);
}

std::size_t serial_device::read(void *data
                                , std::size_t size
                                , timeout_t timeout)
{
    return m_serial_device_context->read(data
                                         , size
                                         , timeout);
}

std::size_t serial_device::read(frame_data_t &frame_data
                                , bool is_append
                                , timeout_t timeout)
{
    return m_serial_device_context->read(frame_data
                                         , is_append
                                         , timeout);
}

frame_data_t serial_device::read(timeout_t timeout)
{
    return m_serial_device_context->read(timeout);
}

bool serial_device::flush()
{
    return m_serial_device_context->flush();
}

std::size_t serial_device::get_ready_data_size()
{
    return m_serial_device_context->get_ready_data_size();
}

}
