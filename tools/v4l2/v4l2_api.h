#ifndef V4L2_API_H
#define V4L2_API_H

#include "v4l2_base.h"

namespace v4l2
{
const std::uint32_t default_try_timeout = 100;

typedef std::int32_t handle_t;

std::int32_t xioctl(handle_t handle
                    , std::int32_t request
                    , void *arg = nullptr
                    , std::uint32_t try_timeout = default_try_timeout);

bool io_wait(handle_t handle
             , std::uint32_t timeout);

handle_t open_device(const std::string& uri_device);
bool close_device(handle_t handle);
format_list_t fetch_supported_format(handle_t handle);
bool fetch_frame_format(handle_t handle
                        , frame_size_t& frame_size
                        , pixel_format_t& pixel_format);
bool fetch_fps(handle_t handle
               , std::uint32_t& fps);

bool set_frame_format(handle_t handle
                      , const frame_size_t& frame_size
                      , pixel_format_t pixel_format);

bool set_fps(handle_t handle
             , std::uint32_t fps);

bool set_control(handle_t handle
                 , std::uint32_t id
                 , std::int32_t value);

bool get_control(handle_t handle
                 , std::uint32_t id
                 , std::int32_t& value);

mapped_buffer_t map(handle_t handle, std::size_t buffer_count = 1);
std::size_t unmap(handle_t handle, mapped_buffer_t& mapped_buffer);

control_map_t fetch_control_list(handle_t handle);
frame_data_t fetch_frame_data(handle_t handle
                              , mapped_buffer_t& mapped_buffer
                              , std::uint32_t timeout = 0);


}

#endif // V4L2_API_H
