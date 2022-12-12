#ifndef SERIAL_API_H
#define SERIAL_API_H

#include "serial_base.h"

namespace serial
{

typedef std::int32_t handle_t;
const handle_t no_handle = -1;

handle_t serial_open(const std::string& sernal_name
                     , const control_config_t& control_config);

bool serial_close(handle_t handle);

bool serial_set_config(handle_t handle
                       , const control_config_t& control_config
                       , bool is_now = true);
bool serial_get_config(handle_t handle
                       , control_config_t& control_config);

wait_status_t io_wait(handle_t handle
                      , timeout_t timeout = infinite_timeout);

bool is_valid_handle(handle_t handle);

std::size_t serial_write(handle_t handle
                         , const void* data
                         , std::size_t size);

std::size_t serial_read(handle_t handle
                        , void* data
                        , std::size_t size
                        , timeout_t timeout = infinite_timeout);

std::size_t serial_read(handle_t handle
                        , frame_data_t& frame_data
                        , bool is_append = false
                        , timeout_t timeout = infinite_timeout);

std::size_t serial_unread_data_size(handle_t handle);

bool serial_flush(handle_t handle);

serial_list_t serial_list();

}

#endif // SERIAL_API_H
