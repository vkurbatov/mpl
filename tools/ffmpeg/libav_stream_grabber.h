#ifndef FFMPEG_LIBAV_STREAM_GRABBER_H
#define FFMPEG_LIBAV_STREAM_GRABBER_H

#include "libav_base.h"

namespace pt::ffmpeg
{

struct libav_stream_grabber_context_t;
using libav_stream_grabber_context_ptr_t = std::shared_ptr<libav_stream_grabber_context_t>;

struct libav_grabber_config_t
{
    std::string     url;
    stream_mask_t   stream_mask;
    std::string     options;
    libav_grabber_config_t(const std::string& url = {}
                           , stream_mask_t stream_mask = stream_mask_t::stream_mask_all
                           , std::string options = {});
};

class libav_stream_grabber
{
    libav_stream_grabber_context_ptr_t m_libav_stream_grabber_context;
    frame_handler_t                     m_frame_handler;
    stream_event_handler_t              m_stream_event_handler;

public:
    libav_stream_grabber(frame_handler_t frame_handler = nullptr
                         , stream_event_handler_t stream_event_handler = nullptr);

    bool open(const libav_grabber_config_t& config);
    bool close();
    bool is_opened() const;
    bool is_established() const;
    bool get_config(libav_grabber_config_t& config);
    bool set_config(const libav_grabber_config_t& config);
    capture_diagnostic_t diagnostic() const;
    stream_info_list_t streams() const;
    frame_queue_t fetch_media_queue(std::int32_t stream_id);
};

}

#endif // ffmpeg_LIBAV_STREAM_GRABBER_H
