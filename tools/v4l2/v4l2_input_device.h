#ifndef V4L2_INPUT_DEVICE_H
#define V4L2_INPUT_DEVICE_H

#include "v4l2_base.h"

namespace v4l2
{

class v4l2_input_device
{
    struct context_t;
    using context_ptr_t = std::unique_ptr<context_t>;

    context_ptr_t       m_context;
public:
    struct config_t
    {
        std::string     url;
        std::size_t     buffers;
        std::uint32_t   read_timeout = 0;
        frame_info_t    frame_info;

        config_t(const std::string_view& url = {}
                , std::size_t buffers = 1
                , std::uint32_t read_timeout = 0
                , const frame_info_t& frame_info = {});
    };

    v4l2_input_device(const config_t& config = {});
    ~v4l2_input_device();

    bool open();
    bool close();
    bool is_opened() const;

    bool set_config(const config_t& config);
    const config_t& config() const;

    frame_info_t::array_t get_supported_formats() const;
    frame_info_t get_format() const;
    bool set_format(const frame_info_t& format);

    control_info_t::map_t get_supported_controls() const;
    std::size_t controls(ctrl_command_t::array_t& controls);

    bool read_frame(frame_t& frame);
};

}

#endif // V4L2_INPUT_DEVICE_H
