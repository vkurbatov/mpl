#ifndef V4L2_DEVICE_H
#define V4L2_DEVICE_H

#include "v4l2_base.h"

namespace v4l2
{

class v4l2_device
{
    struct context_t;
    using context_ptr_t = std::unique_ptr<context_t>;
    context_ptr_t                       m_context;

    frame_handler_t                     m_frame_handler;
    stream_event_handler_t              m_stream_event_handler;
public:

    v4l2_device(frame_handler_t frame_handler = nullptr
                , stream_event_handler_t stream_event_handler = nullptr);

    ~v4l2_device();

    bool open(const std::string& uri
              , std::uint32_t buffer_count = 1);
    bool close();
    bool is_opened() const;
    bool is_established() const;

    format_list_t get_supported_formats() const;
    frame_info_t get_format() const;
    bool set_format(const frame_info_t& format);

    control_list_t get_control_list() const;
    bool set_control(std::uint32_t control_id, std::int32_t value);
    std::int32_t get_control(std::uint32_t control_id, std::int32_t default_value = 0);

    bool set_relative_control(std::uint32_t control_id, double value);
    double get_relatuive_control(std::uint32_t control_id, double default_value = 0);

    bool get_ptz(double& pan, double& tilt, double& zoom);
    bool set_ptz(double pan, double tilt, double zoom);

    frame_queue_t fetch_media_queue();
};

}


#endif // V4L2_DEVICE_H
