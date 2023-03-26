#ifndef V4L2_BASE_H
#define V4L2_BASE_H

#include <string>
#include <vector>
#include <map>
#include <queue>
#include <memory>
#include <functional>

namespace v4l2
{

enum class streaming_event_t
{
    start,
    stop,
    open,
    close
};

typedef std::vector<std::uint8_t> frame_data_t;

extern const std::uint32_t ctrl_tilt_absolute;
extern const std::uint32_t ctrl_pan_absolute;
extern const std::uint32_t ctrl_zoom_absolute;
extern const std::uint32_t ctrl_pan_speed;
extern const std::uint32_t ctrl_tilt_speed;
extern const std::uint32_t ctrl_zoom_speed;

typedef std::uint32_t pixel_format_t;
const pixel_format_t pixel_format_unknown = 0;
extern const pixel_format_t pixel_format_h264;
extern const pixel_format_t pixel_format_jpeg;
extern const pixel_format_t pixel_format_mjpeg;

struct frame_size_t
{
    std::uint32_t width;
    std::uint32_t height;

    frame_size_t(std::uint32_t width
                 , std::uint32_t height);

    bool operator == (const frame_size_t& frame_size) const;
    bool operator != (const frame_size_t& frame_size) const;

    bool is_null() const;
};

struct frame_info_t
{
    using array_t = std::vector<frame_info_t>;
    frame_size_t    size;
    std::uint32_t   fps;
    pixel_format_t  pixel_format;

    frame_info_t(const frame_size_t& size = { 0, 0 }
                 , std::uint32_t fps = 0
                 , pixel_format_t pixel_format = 0);

    bool operator == (const frame_info_t& frame_info) const;
    bool operator != (const frame_info_t& frame_info) const;

    bool is_null() const;

};

struct frame_t
{
    frame_info_t    frame_info;
    frame_data_t    frame_data;

    frame_t(const frame_info_t& frame_info = frame_info_t()
            , const frame_data_t& frame_data = frame_data_t());

    frame_t(const frame_info_t& frame_info
            , frame_data_t&& frame_data);
};

using frame_queue_t = std::queue<frame_t>;
using ctrl_value_t = std::int32_t;
using ctrl_id_t = std::uint32_t;

struct control_range_t
{
    ctrl_value_t   min;
    ctrl_value_t   max;

    control_range_t(ctrl_value_t min
                    , ctrl_value_t max);

    bool operator == (const control_range_t& range) const;
    bool operator != (const control_range_t& range) const;

   ctrl_value_t range_length() const;

    bool is_join(ctrl_value_t value) const;
};

enum control_type_t
{
    undefined,
    numeric,
    boolean,
    menu
};

struct control_menu_t
{
    using array_t = std::vector<control_menu_t>;
    ctrl_id_t       id;
    std::string     name;

    control_menu_t(std::uint32_t id
                        , const std::string& name);
};

struct control_info_t
{
    using array_t = std::vector<control_info_t>;
    using map_t = std::map<ctrl_id_t, control_info_t>;

    ctrl_id_t               id;
    std::string             name;
    ctrl_value_t            step;
    ctrl_value_t            default_value;
    ctrl_value_t            current_value;
    control_range_t         range;
    control_menu_t::array_t menu;

    control_info_t(ctrl_id_t id
                  , const std::string& name
                  , ctrl_value_t step
                  , ctrl_value_t default_value
                  , ctrl_value_t current_value
                  , ctrl_value_t min
                  , ctrl_value_t max);

    control_type_t type() const;
};

struct ctrl_command_t
{
    using array_t = std::vector<ctrl_command_t>;

    ctrl_id_t       id;
    ctrl_value_t    value;
    bool            is_set;
    bool            success;
    std::uint32_t   delay_ms;

    ctrl_command_t(ctrl_id_t id = 0
                   , ctrl_value_t value = 0
                   , bool is_set = false
                   , bool success = false
                   , std::uint32_t delay_ms = 0);
};

struct buffer_item_t
{
    void *buffer;
    std::size_t size;
};

struct mapped_buffer_t
{
    std::vector<buffer_item_t>  buffers;
    std::uint32_t               index = 0;

    buffer_item_t& current();
    void next();
};

using frame_handler_t = std::function<bool(frame_t&& frame)>;

using stream_event_handler_t = std::function<void(const streaming_event_t& streaming_event)>;

}

#endif // V4L2_BASE_H
