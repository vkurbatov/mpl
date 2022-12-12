#ifndef SERIAL_DEVICE_H
#define SERIAL_DEVICE_H

#include "serial_base.h"
#include <memory>

namespace serial
{

struct serial_device_context_t;
struct serial_device_context_deleter_t { void operator()(serial_device_context_t* serial_device_context_ptr); };

typedef std::unique_ptr<serial_device_context_t, serial_device_context_deleter_t>  serial_device_context_ptr_t;

class serial_device
{
    serial_device_context_ptr_t     m_serial_device_context;
public:
    static serial_list_t serial_devices();

    serial_device(const control_config_t& control_config = control_config_t());

    bool open(const std::string& serial);
    bool close();
    bool is_opened() const;
    bool is_established() const;

    const control_config_t& config() const;
    bool set_config(const control_config_t& control_config);

    std::size_t write(const void* data
                      , std::size_t size);

    std::size_t read(void* data
                     , std::size_t size
                     , timeout_t timeout = infinite_timeout);

    std::size_t read(frame_data_t& frame_data
                     , bool is_append
                     , timeout_t timeout = infinite_timeout);

    frame_data_t read(timeout_t timeout = infinite_timeout);

    bool flush();

    std::size_t get_ready_data_size();

};

}

#endif // SERIAL_DEVICE_H
