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

typedef std::vector<frame_info_t> format_list_t;

struct frame_t
{
    frame_info_t    frame_info;
    frame_data_t    frame_data;

    frame_t(const frame_info_t& frame_info = frame_info_t()
            , const frame_data_t& frame_data = frame_data_t());

    frame_t(const frame_info_t& frame_info
            , frame_data_t&& frame_data);
};

typedef std::queue<frame_t> frame_queue_t;

typedef std::int32_t value_type_t;

struct control_range_t
{
    value_type_t   min;
    value_type_t   max;

    control_range_t(value_type_t min
                    , value_type_t max);

    bool operator == (const control_range_t& range) const;
    bool operator != (const control_range_t& range) const;

   value_type_t range_length() const;

    bool is_join(value_type_t value) const;
};

enum control_type_t
{
    undefined,
    numeric,
    boolean,
    menu
};

struct control_menu_item_t
{
    std::int32_t id;
    std::string  name;

    control_menu_item_t(std::uint32_t id
                        , const std::string& name);
};

typedef std::vector<control_menu_item_t> control_menu_t;

struct control_t
{
    std::int32_t    id;
    std::string     name;
    value_type_t    step;
    value_type_t    default_value;
    value_type_t    current_value;
    control_range_t range;
    control_t(std::uint32_t id
              , const std::string& name
              , value_type_t step
              , value_type_t default_value
              , value_type_t current_value
              , value_type_t min
              , value_type_t max);
    control_menu_t  menu;
    control_type_t type() const;
};

typedef std::map<std::uint32_t, control_t> control_map_t;
typedef std::vector<control_t> control_list_t;

struct buffer_item_t
{
    void *buffer;
    std::size_t size;
};

struct mapped_buffer_t
{
    std::vector<buffer_item_t>  buffers;
    std::uint32_t               index;

    buffer_item_t& current();
    void next();
};

typedef std::function<bool(frame_t&& frame)> frame_handler_t;


typedef std::function<bool(const frame_info_t& frame_info
                           , frame_data_t&& frame_data)> stream_data_handler_t;

typedef std::function<void(const streaming_event_t& streaming_event)> stream_event_handler_t;

}

#endif // V4L2_BASE_H
