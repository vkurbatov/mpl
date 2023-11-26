#ifndef FFMPEG_LIBAV_OUTPUT_FORMAT_H
#define FFMPEG_LIBAV_OUTPUT_FORMAT_H

#include "libav_base.h"

namespace pt::ffmpeg
{

class libav_output_format
{
public:
    struct config_t
    {
        std::string             url;
        std::string             options;
        stream_info_t::list_t   streams;

        config_t(const std::string_view& url = {}
                 , const std::string_view& options = {}
                 , const stream_info_t::list_t streams = {});

        bool is_valid() const;

    };
private:
    struct context_t;
    using context_ptr_t = std::unique_ptr<context_t>;

    context_ptr_t       m_context;
public:
    libav_output_format(const config_t& config = {});

    ~libav_output_format();

    const config_t& config() const;
    bool set_config(const config_t& config);

    bool open();
    bool close();

    bool is_open() const;

    bool write(const frame_ref_t& frame);
};

}

#endif // FFMPEG_LIBAV_OUTPUT_FORMAT_H
