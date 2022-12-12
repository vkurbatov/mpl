#ifndef LIBAV_STREAM_PUBLISHER_H
#define LIBAV_STREAM_PUBLISHER_H

#include "libav_base.h"

namespace ffmpeg
{

struct libav_stream_publisher_context_t;
struct libav_stream_publisher_context_deleter_t { void operator()(libav_stream_publisher_context_t* libav_stream_publisher_context_ptr); };

typedef std::unique_ptr<libav_stream_publisher_context_t, libav_stream_publisher_context_deleter_t> libav_stream_publisher_context_ptr_t;

class libav_stream_publisher
{
    libav_stream_publisher_context_ptr_t m_libav_stream_publisher_context;

public:
    libav_stream_publisher();

    bool open(const std::string& uri
              , const stream_info_list_t& stream_list);

    bool close();
    bool is_opened() const;
    bool is_established() const;

    stream_info_list_t streams() const;

    bool push_frame(std::int32_t stream_id
                    , const void* data
                    , std::size_t size
                    , std::int64_t timestamp
                    , bool key_frame = false);

    bool push_frame(const frame_t& frame);

};

}

#endif // LIBAV_STREAM_PUBLISHER_H
