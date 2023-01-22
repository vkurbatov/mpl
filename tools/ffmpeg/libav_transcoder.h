#ifndef FFMPEG_libav_transcoder_H
#define FFMPEG_libav_transcoder_H

#include "libav_base.h"
#include <set>

namespace ffmpeg
{

struct libav_transcoder_context_t;
struct libav_transcoder_context_deleter_t { void operator()(libav_transcoder_context_t* libav_transcoder_context_ptr); };

typedef std::unique_ptr<libav_transcoder_context_t, libav_transcoder_context_deleter_t> libav_transcoder_context_ptr_t;

enum class transcoder_type_t
{
    unknown,
    encoder,
    decoder
};

enum class transcode_flag_t : std::uint32_t
{
    none = 0,
    key_frame = 1
};

using format_set_t = std::set<format_id_t>;

class libav_transcoder
{
    libav_transcoder_context_ptr_t     m_transcoder_context;

public:

    using u_ptr_t = std::unique_ptr<libav_transcoder>;

    static u_ptr_t create();

    libav_transcoder();

    bool open(const stream_info_t& steam_info
              , transcoder_type_t transcoder_type
              , const std::string& options = "");
    bool close();
    bool is_open() const;

    format_set_t supported_formats() const;

    transcoder_type_t type() const;

    const stream_info_t& config() const;

    frame_queue_t transcode(const void* data
                            , std::size_t size
                            , transcode_flag_t transcode_flags = transcode_flag_t::none
                            , std::int64_t timestamp = -1);

    bool transcode(const void* data
                   , std::size_t size
                   , frame_queue_t& frame_queue
                   , transcode_flag_t transcode_flags = transcode_flag_t::none
                   , std::int64_t timestamp = -1);


};

}

#endif // ffmpeg_libav_transcoder_H
