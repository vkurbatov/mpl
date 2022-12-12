#include "serial_api.h"

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <dirent.h>

namespace serial
{

const auto serial_open_flags = O_RDWR | O_NDELAY | O_NOCTTY;
const auto serial_l_flags_disable = (ICANON | ECHO | ECHOE | ISIG);

handle_t serial_open(const std::string &sernal_name
                     , const control_config_t &control_config)
{
    handle_t handle = open(sernal_name.c_str()
                           , serial_open_flags);

    if (serial_set_config(handle
                          , control_config))
    {
        return handle;
    }

    serial_close(handle);

    return no_handle;
}

bool serial_close(handle_t handle)
{
    return is_valid_handle(handle)
            && close(handle) >= 0;
}

bool serial_set_config(handle_t handle
                       , const control_config_t &control_config
                       , bool is_now)
{
    if (is_valid_handle(handle))
    {
        struct termios serial_options = {};

        if (tcgetattr(handle
                      , &serial_options) >= 0)
        {
            control_config.update_flags(serial_options.c_cflag);
            serial_options.c_lflag &= ~(serial_l_flags_disable);
            serial_options.c_iflag = 0;
            serial_options.c_oflag &= ~OPOST;

            if (tcsetattr(handle
                          , is_now ? TCSANOW : TCSADRAIN
                          , &serial_options) >= 0)
            {
                return true;
            }
        }
    }

    return false;
}

bool serial_get_config(handle_t handle
                       , control_config_t &control_config)
{
    if (is_valid_handle(handle))
    {
        struct termios serial_options = {};

        if (tcgetattr(handle
                      , &serial_options) >= 0)
        {
            control_config.set_flags(serial_options.c_cflag);
            return true;
        }
    }

    return false;
}

wait_status_t io_wait(handle_t handle
                      , timeout_t timeout)
{
    fd_set rfds{};

    struct timeval tv = { timeout / 1000
                          , (timeout % 1000) * 1000 };
    FD_ZERO(&rfds);
    FD_SET(handle, &rfds);

    auto result =  select(handle + 1
                         , &rfds
                         , nullptr
                         , nullptr
                         , timeout == infinite_timeout
                            ? nullptr
                            : &tv);

    return result < 0
            ? wait_status_t::error
            : result == 0
              ? wait_status_t::timeout
              : wait_status_t::success;
}

bool is_valid_handle(handle_t handle)
{
    return handle >= 0;
}

std::size_t serial_write(handle_t handle
                         , const void *data
                         , std::size_t size)
{

    if (is_valid_handle(handle))
    {
        auto result = write(handle
                            , data
                            , size);

        return result > 0
                ? result
                : 0;
    }

    return 0;
}

std::size_t serial_read(handle_t handle
                        , void *data
                        , std::size_t size
                        , timeout_t timeout)
{
    if (is_valid_handle(handle))
    {
        if (timeout != no_timeout)
        {
            if (io_wait(handle
                        , timeout) != wait_status_t::success)
            {
                return 0;
            }

            auto result = read(handle
                               , data
                               , size);

            return result > 0
                            ? result
                            : 0;

        }
    }

    return 0;
}

std::size_t serial_read(handle_t handle
                        , frame_data_t &frame_data
                        , bool is_append
                        , timeout_t timeout)
{
    if (is_valid_handle(handle))
    {
        if (timeout != no_timeout)
        {
            if (io_wait(handle
                        , timeout) != wait_status_t::success)
            {
                return 0;
            }
        }

        auto need_size = serial_unread_data_size(handle);

        if (need_size > 0)
        {
            std::uint8_t* data = frame_data.data();

            if (is_append)
            {
                frame_data.resize(frame_data.size() + need_size);
                data = frame_data.data() + frame_data.size() - need_size;
            }
            else
            {
                frame_data.resize(need_size);
                data = frame_data.data();
            }

            auto result = read(handle
                               , data
                               , need_size);

            return result > 0
                            ? result
                            : 0;
        }
    }

    return 0;
}

bool serial_flush(handle_t handle)
{
    return is_valid_handle(handle)
            && tcflush(handle
                       , TCIOFLUSH) >= 0;
}

std::size_t serial_unread_data_size(handle_t handle)
{
    if (is_valid_handle(handle))
    {
        std::uint32_t unread_data_size = 0;

        if (ioctl(handle, FIONREAD, &unread_data_size) >= 0)
        {
            return unread_data_size;
        }
    }

    return 0;
}

serial_list_t serial_list()
{
    serial_list_t serial_list;
    auto dev = opendir("/dev");


    if (dev != nullptr)
    {
        do
        {
            auto dir_entry = readdir(dev);

            if (dir_entry == nullptr)
            {
                break;
            }

            std::string device_name = dir_entry->d_name;

            if (device_name.find("ttyU") == 0)
            {
                serial_list.emplace_back("/dev/" + device_name);
            }
        }
        while(true);

        for (auto i = 0; i < 10; i++)
        {
            serial_list.emplace_back("/dev/ttyS" + std::to_string(i));
        }

        closedir(dev);
    }

    return serial_list;
}


}
