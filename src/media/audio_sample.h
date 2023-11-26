#ifndef MPL_AUDIO_SAMPLE_H
#define MPL_AUDIO_SAMPLE_H

#include "utils/smart_buffer.h"
#include "audio_types.h"
#include "audio_info.h"

namespace mpl::media
{

struct audio_sample_t
{
    audio_info_t        sample_info;
    smart_buffer        sample_data;

    audio_sample_t(const audio_info_t& sample_info = {}
                 , smart_buffer&& sample_data = {});

    audio_sample_t(const audio_info_t& sample_info
                   , const smart_buffer& sample_data);

    bool operator == (const audio_sample_t& other) const;
    bool operator != (const audio_sample_t& other) const;

    std::size_t samples() const;

    void resize(std::size_t new_samples);

    const void* data() const;
    void* data();

    bool clear();

    bool is_valid() const;
    bool is_empty() const;
};

}

#endif // MPL_AUDIO_SAMPLE_H
