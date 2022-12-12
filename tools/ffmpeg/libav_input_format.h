#ifndef LIBAV_INPUT_FORMAT_H
#define LIBAV_INPUT_FORMAT_H

#include "libav_base.h"

namespace ffmpeg
{

struct libav_input_format_context_t;
struct libav_input_format_context_deleter_t { void operator()(libav_input_format_context_t* libav_input_format_context_ptr); };

typedef std::unique_ptr<libav_input_format_context_t, libav_input_format_context_deleter_t> libav_input_format_context_ptr_t;

class libav_input_format
{
    libav_input_format_context_ptr_t m_libav_input_format_context;
public:
    libav_input_format();
    bool open(const std::string& uri
              , const std::string& options = {});
    bool close();
    bool is_opened() const;
    bool is_established() const;
    stream_info_list_t streams() const;
    bool fetch_frame(frame_t& frame);
};

}

#endif // LIBAV_INPUT_FORMAT_H
