#include "v4l2_api.h"

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include <linux/videodev2.h>

#include <chrono>


namespace v4l2
{

namespace utils
{

    const frame_size_t frame_size_preset[] =
    {
        {128,  96},
        {176,  120},
        {176,  144},
        {352,  240},
        {352,  288},
        {704,  576},
        {704,  480},
        {1280, 720},
        {1600, 900},
        {1408, 1152},
        {1920, 1080},
        {2048, 1080},
        {2560, 1440},
        {4096, 2160},
        {4096, 2304}
    };

    std::size_t fetch_supported_fps(handle_t handle
                                    , format_list_t& format_list
                                    , const frame_size_t& frame_size
                                    , pixel_format_t pixel_format)
    {
        std::size_t result = 0;
        struct v4l2_frmivalenum fival = {};

        fival.index = 0;
        fival.pixel_format = pixel_format;
        fival.width = frame_size.width;
        fival.height = frame_size.height;

        while (xioctl(handle, VIDIOC_ENUM_FRAMEINTERVALS, &fival) >= 0)
        {
            switch(fival.type)
            {
                case V4L2_FRMIVAL_TYPE_DISCRETE:
                    format_list.push_back({frame_size
                                           , fival.discrete.denominator / fival.discrete.numerator
                                           , pixel_format});
                    result++;
                break;

                case V4L2_FRMIVAL_TYPE_STEPWISE:
                case V4L2_FRMIVAL_TYPE_CONTINUOUS:
                {
                    std::int32_t f_max = fival.stepwise.min.denominator / fival.stepwise.min.numerator,
                                 f_min = fival.stepwise.max.denominator / fival.stepwise.max.numerator;
                    while (f_max > f_min)
                    {
                        format_list.push_back({frame_size
                                       , f_max
                                       , pixel_format});
                        f_max -= 5;
                        result++;
                    }
                }
                break;
            }

            fival.index++;
        }

        return result;
    }

    std::size_t fetch_supported_frame_sizes(handle_t handle
                                            , format_list_t& format_list
                                            , pixel_format_t pixel_format)
    {
        std::size_t result = 0;

        struct v4l2_frmsizeenum fsize = {};

        fsize.index = 0;
        fsize.pixel_format = pixel_format;

        while (xioctl(handle, VIDIOC_ENUM_FRAMESIZES, &fsize) >= 0)
        {
            switch(fsize.type)
            {
                case V4L2_FRMSIZE_TYPE_DISCRETE:
                    result += fetch_supported_fps(handle
                                                  , format_list
                                                  , { fsize.discrete.width, fsize.discrete.height }
                                                  , pixel_format);
                break;

                case V4L2_FRMSIZE_TYPE_CONTINUOUS:
                case V4L2_FRMSIZE_TYPE_STEPWISE:
                {
                    for (const auto& frame_size : frame_size_preset)
                    {
                        if (fsize.stepwise.min_width < frame_size.width && frame_size.width <= fsize.stepwise.max_width &&
                                                fsize.stepwise.min_height < frame_size.height && frame_size.height <= fsize.stepwise.max_width)
                        {
                            result += fetch_supported_fps(handle
                                                          , format_list
                                                          , { fsize.discrete.width, fsize.discrete.height }
                                                          , pixel_format);
                        }
                    }
                }
                break;
            }

            fsize.index++;
        }

        return result;
    }

    std::size_t fetch_supported_formats(handle_t handle
                                        , format_list_t& format_list)
    {
        std::size_t result = 0;

        struct v4l2_fmtdesc fmt = {};

        fmt.index = 0;
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        while (xioctl(handle, VIDIOC_ENUM_FMT, &fmt) >= 0)
        {
            result += fetch_supported_frame_sizes(handle
                                                  , format_list
                                                  , fmt.pixelformat);

            fmt.index++;
        }

        return result;
    }
}
// -------------------------------------------------------------------------------
handle_t open_device(const std::string& uri_device)
{
    return open(uri_device.c_str(), O_RDWR);
}

bool close_device(int32_t handle)
{
    return handle >= 0
            ? close(handle) >= 0
            : false;
}

int32_t xioctl(handle_t handle
               , int32_t request
               , void *arg
               , std::uint32_t try_timeout)
{
    std::int32_t  result;

    auto tp = std::chrono::high_resolution_clock::now();

    do
    {
        result = ioctl(handle, request, arg) < 0 ? -errno : 0;

        if (result == -EINTR)
        {
            continue;
        }

        auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - tp).count();

        if (dt >= try_timeout)
        {
            break;
        }

    }
    while (result == -EBUSY
           || result == -EAGAIN);

    return result;
}

bool io_wait(handle_t handle
             , uint32_t timeout)
{
    fd_set rfds;
    struct timeval tv;
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;
    FD_ZERO(&rfds);
    FD_SET(handle, &rfds);

    return select(handle + 1, &rfds, NULL, NULL, &tv) > 0;
}

format_list_t fetch_supported_format(handle_t handle)
{
    format_list_t format_list;

    utils::fetch_supported_formats(handle
                                   , format_list);

    return std::move(format_list);
}

bool fetch_frame_format(handle_t handle
                        , frame_size_t &frame_size
                        , pixel_format_t &pixel_format)
{
    struct v4l2_format format = {};
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (xioctl(handle, VIDIOC_G_FMT, &format) >= 0)
    {
        frame_size.width = format.fmt.pix.width;
        frame_size.height = format.fmt.pix.height;
        pixel_format = format.fmt.pix.pixelformat;

        return true;
    }

    return false;
}

bool fetch_fps(handle_t handle
               , uint32_t& fps)
{
    struct v4l2_streamparm streamparam = {};
    streamparam.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (xioctl(handle, VIDIOC_G_PARM, &streamparam) >= 0)
    {
        const auto& time_frame = streamparam.parm.capture.timeperframe;

        fps = time_frame.numerator > 0
                ? time_frame.denominator / time_frame.numerator
                : time_frame.denominator;

        return true;
    }

    return false;
}

control_map_t fetch_control_list(handle_t handle)
{
    control_map_t control_list;

    struct v4l2_query_ext_ctrl queryctrl = {};
    queryctrl.id = V4L2_CTRL_CLASS_USER | V4L2_CTRL_FLAG_NEXT_CTRL | V4L2_CTRL_FLAG_GRABBED;

    while (xioctl(handle, VIDIOC_QUERY_EXT_CTRL, &queryctrl) >= 0)
    {
        struct v4l2_control c = {};
        c.id = queryctrl.id;

        if ((queryctrl.flags & (V4L2_CTRL_FLAG_DISABLED | V4L2_CTRL_FLAG_READ_ONLY)) == 0
                && xioctl(handle, VIDIOC_G_CTRL, &c) >= 0)
        {
            control_t control(c.id
                              , queryctrl.name
                              , queryctrl.step
                              , queryctrl.default_value
                              , c.value
                              , queryctrl.minimum
                              , queryctrl.maximum
                              );
            if (queryctrl.type == V4L2_CTRL_TYPE_MENU)
            {
                struct v4l2_querymenu querymenu = {};
                querymenu.id = queryctrl.id;

                for (querymenu.index = queryctrl.minimum;
                                    querymenu.index <= queryctrl.maximum;
                                    querymenu.index++)
                {
                    if (xioctl(handle, VIDIOC_QUERYMENU, &querymenu) >= 0)
                    {
                        control.menu.push_back({ querymenu.index
                                                 , reinterpret_cast<const char*>(querymenu.name) });
                    }
                }
            }

            control_list.emplace(control.id, std::move(control));
        }

        queryctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;

    }

    return std::move(control_list);
}

bool set_frame_format(handle_t handle
                      , const frame_size_t &frame_size
                      , pixel_format_t pixel_format)
{
    struct v4l2_format video_fmt = {};

    video_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    video_fmt.fmt.pix.width = frame_size.width;
    video_fmt.fmt.pix.height = frame_size.height;
    video_fmt.fmt.pix.pixelformat = pixel_format;   

    return xioctl(handle, VIDIOC_S_FMT, &video_fmt) >= 0;
}

bool set_fps(handle_t handle
             , uint32_t fps)
{    
    struct v4l2_streamparm stream_parm = {};
    stream_parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    stream_parm.parm.capture.timeperframe.numerator = 1000;
    stream_parm.parm.capture.timeperframe.denominator = fps * 1000;

    return xioctl(handle, VIDIOC_S_PARM, &stream_parm) >= 0;
}

mapped_buffer_t map(handle_t handle, std::size_t buffer_count)
{
    mapped_buffer_t mapped_buffer;

    mapped_buffer.index = 0;

    struct v4l2_requestbuffers reqbuf = {};
    reqbuf.count = buffer_count;
    reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqbuf.memory = V4L2_MEMORY_MMAP;   

    if (xioctl(handle, VIDIOC_REQBUFS, &reqbuf) >= 0)
    {
        bool has_error = false;

        for (std::uint32_t idx = 0; idx < reqbuf.count && !has_error; ++idx)
        {
            has_error = true;

            struct v4l2_buffer buffer = {};
            buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buffer.memory = V4L2_MEMORY_MMAP;
            buffer.index = idx;         

            if (xioctl(handle, VIDIOC_QUERYBUF, &buffer) >= 0)
            {
                void* mem_buffer = mmap(0
                                        , buffer.length
                                        , PROT_READ | PROT_WRITE
                                        , MAP_SHARED
                                        , handle
                                        , buffer.m.offset);
                if (mem_buffer != MAP_FAILED
                        && xioctl(handle, VIDIOC_QBUF, &buffer) >= 0)
                {
                    mapped_buffer.buffers.push_back({ mem_buffer, buffer.length });
                    has_error = false;
                }
            }
        }

        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        if (has_error || xioctl(handle, VIDIOC_STREAMON, &type) < 0)
        {
            unmap(handle, mapped_buffer);
            mapped_buffer.buffers.clear();
        }
    }

    return std::move(mapped_buffer);
}

std::size_t unmap(handle_t handle, mapped_buffer_t &mapped_buffer)
{
    std::size_t result = 0;

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    xioctl(handle, VIDIOC_STREAMOFF, &type);

    for (auto& buffer : mapped_buffer.buffers)
    {
        if (munmap(buffer.buffer, buffer.size) >= 0)
        {
            result++;
        }
    }

    mapped_buffer.buffers.clear();
    mapped_buffer.index = 0;

    return result;
}

frame_data_t fetch_frame_data(handle_t handle
                              , mapped_buffer_t& mapped_buffer
                              , std::uint32_t timeout)
{
    frame_data_t frame_data;

    if (timeout == 0
            || io_wait(handle, timeout))
    {
        struct v4l2_buffer buffer = {0};

        buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buffer.memory = V4L2_MEMORY_MMAP;
        buffer.index = mapped_buffer.index;

        if (xioctl(handle, VIDIOC_DQBUF, &buffer) >= 0)
        {
            auto current = mapped_buffer.current();
            auto data = static_cast<const std::uint8_t*>(current.buffer);
            if (buffer.bytesused > 0
                    && buffer.bytesused <= current.size)
            {
                frame_data = std::move(frame_data_t(data
                                                    , data + buffer.bytesused));
                mapped_buffer.next();


            }

            xioctl(handle, VIDIOC_QBUF, &buffer);
        }
    }

    return std::move(frame_data);
}

bool set_control(handle_t handle, uint32_t id, int32_t value)
{
    struct v4l2_control v_control = {};
    v_control.id = id;
    v_control.value = value;

    return xioctl(handle, VIDIOC_S_CTRL, &v_control) >= 0;
}

bool get_control(handle_t handle, uint32_t id, int32_t &value)
{
    struct v4l2_control v_control = {};
    v_control.id = id;

    if (xioctl(handle, VIDIOC_G_CTRL, &v_control) >= 0)
    {
        value = v_control.value;
        return true;
    }

    return false;
}


}
