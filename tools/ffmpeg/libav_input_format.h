#ifndef FFMPEG_LIBAV_INPUT_FORMAT_H
#define FFMPEG_LIBAV_INPUT_FORMAT_H

#include "libav_base.h"

namespace ffmpeg
{

class libav_input_format
{
public:
    struct config_t
    {
        std::string         url;
        std::string         options;

        config_t(const std::string_view& url = {}
                 , const std::string_view& options = {});

        bool is_valid() const;
    };
private:
    struct context_t;
    using context_ptr_t = std::unique_ptr<context_t>;

    context_ptr_t       m_context;

public:
    libav_input_format(const config_t& config = {});

    ~libav_input_format();

    const config_t& config() const;
    bool set_config(const config_t& config);

    stream_info_list_t streams() const;

    bool open();
    bool close();

    bool is_open() const;
    bool cancel();

    bool read(frame_t& frame);
    bool read(frame_ref_t& ref_frame);
};

}

#endif // FFMPEG_LIBAV_INPUT_FORMAT_H
