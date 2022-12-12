#ifndef LIBAV_RESAMPLER_H
#define LIBAV_RESAMPLER_H

#include "libav_base.h"

namespace ffmpeg
{

struct resampler_context_t;

class libav_resampler
{
    using resampler_context_ptr_t = std::shared_ptr<resampler_context_t>;
    resampler_context_ptr_t m_resampler_context;
public:
    libav_resampler();
    media_data_t resample(const audio_info_t& input_format
                  , const void* input_data
                  , std::size_t input_size
                  , const audio_info_t& output_format);
};

}

#endif // LIBAV_RESAMPLER_H
